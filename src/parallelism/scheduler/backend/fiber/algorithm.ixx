module;

#include <boost/fiber/algo/algorithm.hpp>
#include <boost/fiber/scheduler.hpp>
#include <boost/fiber/detail/context_spinlock_queue.hpp>

export module synodic.soul.engine.fiber:algorithm;

import std;
import :properties;

export class SchedulerAlgorithm :
	public boost::fibers::algo::algorithm_with_properties<Properties> {

public:

	typedef boost::fibers::scheduler::ready_queue_type localQueueType;
	typedef boost::fibers::detail::context_spinlock_queue queueType;	//TODO: replace with custom spinlock queue variety


	//Construction

	SchedulerAlgorithm(std::uint32_t, bool = false);
	~SchedulerAlgorithm() = default;

	SchedulerAlgorithm(SchedulerAlgorithm const&) = delete;
	SchedulerAlgorithm(SchedulerAlgorithm &&) = delete;

	SchedulerAlgorithm& operator=(SchedulerAlgorithm const&) = delete;
	SchedulerAlgorithm& operator=(SchedulerAlgorithm &&) = delete;


	//Implementation
	void								awakened(boost::fibers::context*, Properties&)			noexcept override;
	inline boost::fibers::context*		PickShared(std::uint32_t)											noexcept;
	inline boost::fibers::context*		PickLocal(std::uint32_t)												noexcept;
	boost::fibers::context*				pick_next()													noexcept override;
	bool								has_ready_fibers()											const noexcept override;
	void								property_change(boost::fibers::context*, Properties&)  noexcept override;
	void								suspend_until(std::chrono::steady_clock::time_point const&) noexcept override;
	void								notify()													noexcept override;


private:

	static void InitializeSchedulers(std::uint32_t);

	static std::atomic<std::uint32_t>										counter_;
	static std::vector<boost::intrusive_ptr<SchedulerAlgorithm>>    schedulers_;

	static thread_local std::minstd_rand generator_;
	static std::uniform_int_distribution<std::uint32_t> distribution_;

	std::uint32_t                                        id_;
	std::uint32_t                                        threadCount_;

	boost::fibers::detail::spinlock				localLocks_[3]; //TODO: replace with a custom spinlock varient
	localQueueType								localQueues_[3];
	queueType									sharedQueues_[3];

	std::mutex              sleepMutex_;
	std::condition_variable sleepCondition_;
	bool                    sleepFlag_;

	bool                    suspend_; //scheduler will sleep when no work is found. Needs an externel wakeup if so

};
