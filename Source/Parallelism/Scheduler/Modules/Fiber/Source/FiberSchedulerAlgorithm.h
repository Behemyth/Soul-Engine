#pragma once

#include "FiberProperties.h"
#include "Types.h"

#include <boost/fiber/algo/algorithm.hpp>
#include <boost/fiber/scheduler.hpp>
#include <boost/fiber/detail/context_spinlock_queue.hpp>

#include <vector>

class FiberSchedulerAlgorithm :
	public boost::fibers::algo::algorithm_with_properties<FiberProperties> {

public:

	typedef boost::fibers::scheduler::ready_queue_type localQueueType;  
	typedef boost::fibers::detail::context_spinlock_queue queueType;	//TODO: replace with custom spinlock queue variety


	//Construction

	FiberSchedulerAlgorithm(uint, bool = false);
	~FiberSchedulerAlgorithm() = default;

	FiberSchedulerAlgorithm(FiberSchedulerAlgorithm const&) = delete;
	FiberSchedulerAlgorithm(FiberSchedulerAlgorithm &&) = delete;

	FiberSchedulerAlgorithm& operator=(FiberSchedulerAlgorithm const&) = delete;
	FiberSchedulerAlgorithm& operator=(FiberSchedulerAlgorithm &&) = delete;


	//Implementation
	void								awakened(boost::fibers::context*, FiberProperties&)			noexcept override;
	inline boost::fibers::context*		PickShared(uint)											noexcept;
	inline boost::fibers::context*		PickLocal(uint)												noexcept;
	boost::fibers::context*				pick_next()													noexcept override;
	bool								has_ready_fibers()											const noexcept override;
	void								property_change(boost::fibers::context*, FiberProperties&)  noexcept override;
	void								suspend_until(std::chrono::steady_clock::time_point const&) noexcept override;
	void								notify()													noexcept override;


private:

	static void InitializeSchedulers(uint);

	static std::atomic<uint>										counter_;
	static std::vector<boost::intrusive_ptr<FiberSchedulerAlgorithm>>    schedulers_;

	static thread_local std::minstd_rand generator_;
	static std::uniform_int_distribution<uint> distribution_;

	uint                                        id_;
	uint                                        threadCount_;

	boost::fibers::detail::spinlock				localLocks_[3]; //TODO: replace with a custom spinlock varient
	localQueueType								localQueues_[3];
	queueType									sharedQueues_[3];

	std::mutex              sleepMutex_;
	std::condition_variable sleepCondition_;
	bool                    sleepFlag_;

	bool                    suspend_; //scheduler will sleep when no work is found. Needs an externel wakeup if so

};
