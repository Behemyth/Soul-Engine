#include "FiberSchedulerAlgorithm.h"

#include <boost/context/detail/prefetch.hpp>
#include <boost/fiber/mutex.hpp>

std::atomic<uint> FiberSchedulerAlgorithm::counter_(0);
std::vector<boost::intrusive_ptr<FiberSchedulerAlgorithm>> FiberSchedulerAlgorithm::schedulers_;

thread_local std::minstd_rand FiberSchedulerAlgorithm::generator_;
std::uniform_int_distribution<uint> FiberSchedulerAlgorithm::distribution_;

FiberSchedulerAlgorithm::FiberSchedulerAlgorithm(uint threadCount, bool suspend) :
	id_(counter_++),
	threadCount_(threadCount),
	sleepFlag_(false),
	suspend_(suspend)
{

	//only initialize schedulers once
	static std::once_flag flag;
	std::call_once(flag, &FiberSchedulerAlgorithm::InitializeSchedulers, threadCount_);

	// register this scheduler
	schedulers_[id_] = this;

}

void FiberSchedulerAlgorithm::InitializeSchedulers(uint thread_count) {

	schedulers_.resize(thread_count, nullptr);
	std::random_device r;
	generator_ = std::minstd_rand(r());
	distribution_ = std::uniform_int_distribution<uint>(0, static_cast<uint>(thread_count - 1));

}

//called when a newly `posted` launched, blocked, or yielded fiber wakes up.
void FiberSchedulerAlgorithm::awakened(boost::fibers::context* ctx, FiberProperties& props) noexcept {

	//if the fiber is a worker, open the posibility it may be moved in between threads, so detach it from its current
	if (!ctx->is_context(boost::fibers::type::pinned_context)) {
		ctx->detach();
	}

	//a pinned context can only run on its thread of origin, so push the fiber to the thread only queue, no properties set
	//a dispatched fiber will only apear if it has yeilded, and will not have any user set properties
	//a posted fiber will join the sharedQueues_ only when properties are set, and are pushed to the local in the meantime


	switch (props.GetPriority()) {
	case TaskPriority::LOW:
	{
		boost::fibers::detail::spinlock_lock spinLock(localLocks_[2]);
		localQueues_[2].push_back(*ctx);
	}
	break;
	case TaskPriority::HIGH:
	{
		boost::fibers::detail::spinlock_lock spinLock(localLocks_[1]);
		localQueues_[1].push_back(*ctx);
	}
	break;
	case TaskPriority::UX:
	{
		boost::fibers::detail::spinlock_lock spinLock(localLocks_[0]);
		localQueues_[0].push_back(*ctx);
	}
	break;
	}

	//TODO:: if execution asserts, write/investigate this edgecase
	assert(!ctx->is_context(boost::fibers::type::none));

}

//pick a fiber from the local queue
boost::fibers::context* FiberSchedulerAlgorithm::PickLocal(uint index) noexcept {

	boost::fibers::context* victim;

	{
		boost::fibers::detail::spinlock_lock spinLock(localLocks_[index]);

		if (localQueues_[index].empty()) {
			victim = nullptr;
		}
		else {
			victim = &localQueues_[index].front();
			localQueues_[index].pop_front();
		}
	}

	return victim;
}

//pick a fiber from the shared queue
boost::fibers::context* FiberSchedulerAlgorithm::PickShared(uint index) noexcept {

	boost::fibers::context* victim = sharedQueues_[index].pop();

	if (!victim && threadCount_ > 1) {

		uint id;
		uint count = 0;

		do {

			do {
				++count;

				// random selection of one logical cpu
				id = distribution_(generator_);

				// prevent stealing from own scheduler unless it is the only scheduler
			} while (id == id_);

			//steal context from other scheduler
			if (schedulers_[id]) {
				victim = schedulers_[id]->sharedQueues_[index].steal();
			}

		} while (nullptr == victim && count < schedulers_.size());

	}

	return victim;

}

//picks the fiber to run next
boost::fibers::context* FiberSchedulerAlgorithm::pick_next() noexcept {

	boost::fibers::context* victim = nullptr;

	//interleave local and shared queues. Local are picked first as not to starve them
	for (auto i = 0; i < 3 && !victim; ++i) {

		victim = PickLocal(i);

		if (victim) {
			break;
		}

		victim = PickShared(i);

	}

	if (victim) {

		boost::context::detail::prefetch_range(victim, sizeof(boost::fibers::context));

		//associate the fiber to this thread if it is a worker thread
		if (!victim->is_context(boost::fibers::type::pinned_context)) {
			boost::fibers::context::active()->attach(victim);
		}

	}

	return victim;

}

bool FiberSchedulerAlgorithm::has_ready_fibers() const noexcept {

	return
		!sharedQueues_[0].empty() &&
		!sharedQueues_[1].empty() &&
		!sharedQueues_[2].empty() &&
		!localQueues_[0].empty() &&
		!localQueues_[1].empty() &&
		!localQueues_[2].empty();

}

//all post fibers appear here, ready to be delegated to sharedQueues_
void FiberSchedulerAlgorithm::property_change(boost::fibers::context* ctx, FiberProperties& props) noexcept {

	//possibly changed when not in the local queue, no update needed
	if (!ctx->ready_is_linked()) {
		return;
	}

	//unlinking the context from the local queues, no lock needed as it exists per thread (fiber garunteed to not switch until properties are set)
	ctx->ready_unlink();

	//ready, push fiber to shared queues
	if (const auto requiredThread = props.RequiredThread(); requiredThread >= 0) {

		switch (props.GetPriority()) {
		case TaskPriority::LOW:
		{
			boost::fibers::detail::spinlock_lock spinLock(schedulers_[requiredThread]->localLocks_[2]);
			schedulers_[requiredThread]->localQueues_[2].push_back(*ctx);
		}
		break;
		case TaskPriority::HIGH:
		{
			boost::fibers::detail::spinlock_lock spinLock(schedulers_[requiredThread]->localLocks_[1]);
			schedulers_[requiredThread]->localQueues_[1].push_back(*ctx);
		}
		break;
		case TaskPriority::UX:
		{
			boost::fibers::detail::spinlock_lock spinLock(schedulers_[requiredThread]->localLocks_[0]);
			schedulers_[requiredThread]->localQueues_[0].push_back(*ctx);
		}
		break;
		}

		//because the fiber may be pushed to a different scheduler, notify the target scheduler
		schedulers_[requiredThread]->notify();

	}
	else {

		switch (props.GetPriority()) {
		case TaskPriority::LOW:
			sharedQueues_[2].push(ctx);
			break;
		case TaskPriority::HIGH:
			sharedQueues_[1].push(ctx);
			break;
		case TaskPriority::UX:
			sharedQueues_[0].push(ctx);
			break;
		}


		//hueristic for enabling another scheduler
		//pick one scheduler at random to wake up and try to find work
		if (threadCount_ > 1) {

			uint id;
			do {
				// random selection of one logical cpu
				id = distribution_(generator_);

				// prevent stealing from own scheduler unless it is the only scheduler
			} while (id == id_);

			schedulers_[id]->notify();

		}
	}

}

void FiberSchedulerAlgorithm::suspend_until(std::chrono::steady_clock::time_point const& targetTime) noexcept {

	if (suspend_) {

		if ((std::chrono::steady_clock::time_point::max)() == targetTime) {

			//indefinite sleep
			std::unique_lock< std::mutex > lk(sleepMutex_);
			sleepCondition_.wait(lk, [this]()
			{
				return sleepFlag_;
			});

			sleepFlag_ = false;

		}
		else {

			//timed sleep
			std::unique_lock< std::mutex > lk(sleepMutex_);
			sleepCondition_.wait_until(lk, targetTime, [this]()
			{
				return sleepFlag_;
			});

			sleepFlag_ = false;

		}

	}

}

void FiberSchedulerAlgorithm::notify() noexcept {

	if (suspend_) {

		{
			std::scoped_lock< std::mutex > lk(sleepMutex_);
			sleepFlag_ = true;
		}

		sleepCondition_.notify_all();

	}

}