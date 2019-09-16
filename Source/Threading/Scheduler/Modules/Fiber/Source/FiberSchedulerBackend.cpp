#include "FiberSchedulerBackend.h"
#include "FiberSchedulerAlgorithm.h"
#include "boost/fiber/barrier.hpp"

FiberSchedulerBackend::FiberSchedulerBackend(Property<uint>& threadCount) :
	shouldRun_(true),
	fiberCount_(0),
	threadCount_(threadCount)
{

	boost::fibers::use_scheduling_algorithm<FiberSchedulerAlgorithm>(threadCount_, true);

	//the main thread takes up one slot
	childThreads_.resize(threadCount_ - 1);

	for (auto& thread : childThreads_) {
		thread = std::thread([this] {
			ThreadRun();
		});
	}

	//init the main fiber specifics
	InitPointers();

	{
		std::scoped_lock<boost::fibers::mutex> incrementLock(fiberMutex_);
		fiberCount_++;
	}

	//suprisingly, the main fiber does not need to run on main.
	boost::this_fiber::properties<FiberProperties>().SetProperties(TaskPriority::UX, -1);

}

FiberSchedulerBackend::~FiberSchedulerBackend() {

	int polledCount;
	{
		std::scoped_lock<boost::fibers::mutex> lock(fiberMutex_);
		shouldRun_ = false;
		--fiberCount_;
		polledCount = fiberCount_;
	}

	//all work is done, so notify all waiting threads
	if (polledCount == 0) {

		fiberCondition_.notify_all();

	}
	//wait until all remaining work is done before spinlocking on join. Leave this thread available for processing
	else {

		std::unique_lock<boost::fibers::mutex> lock(fiberMutex_);
		fiberCondition_.wait(lock, [this]()
		{
			return 0 == fiberCount_ && !shouldRun_;
		});

	}

	//join all complete threads
	for (auto& thread : childThreads_) {
		thread.join();
	}

}

/* Initialize the fiber specific stuff. */
//TODO use fiber specific allocator
void FiberSchedulerBackend::InitPointers() {

	if (!blockMutex_.get()) {
		blockMutex_.reset(new boost::fibers::mutex);
	}
	if (!blockCount_.get()) {
		blockCount_.reset(new uint(0));
	}
	if (!blockCondition_.get()) {
		blockCondition_.reset(new boost::fibers::condition_variable);
	}

}


//block the current thread until all registered children complete
void FiberSchedulerBackend::Block() const {

	//get the current fibers stats for blocking
	uint* holdSize = blockCount_.get();

	std::unique_lock<boost::fibers::mutex> lock(*blockMutex_);
	blockCondition_->wait(lock, [holdSize]()
	{
		return 0 == *holdSize;
	});

}

//Yield the current fiber, allowing for another to take its place.
void FiberSchedulerBackend::Yield() {

	boost::this_fiber::yield();

}

/*
*    launches a thread that waits with a fiber conditional, meaning it still executes fibers
*    while waiting for a notify release.
*/

void FiberSchedulerBackend::ThreadRun() {

	boost::fibers::use_scheduling_algorithm<FiberSchedulerAlgorithm>(threadCount_, true);
	boost::this_fiber::properties<FiberProperties>().SetProperties(TaskPriority::LOW, -1);

	//continue processing until exit is called
	{
		std::unique_lock<boost::fibers::mutex> lock(fiberMutex_);
		fiberCondition_.wait(lock, [this]()
		{
			return 0 == fiberCount_ && !shouldRun_;
		});
	}

}