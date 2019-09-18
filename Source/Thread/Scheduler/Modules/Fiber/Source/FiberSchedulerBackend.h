#pragma once

#include <boost/fiber/fss.hpp>
#include <boost/fiber/condition_variable.hpp>

#include <vector>
#include <thread>
#include <mutex>
#include <forward_list>

#include "Property.h"
#include "Parallelism/Scheduler/TaskParameters.h"
#include "FiberProperties.h"
#include "Types.h"

//TODO: unlink the windows headers from the project/cmake https://github.com/Synodic-Software/Soul-Engine/issues/62
#undef CreateWindow
#undef Yield


class FiberSchedulerBackend {

public:

	FiberSchedulerBackend(Property<uint>&);
	~FiberSchedulerBackend();

	FiberSchedulerBackend(FiberSchedulerBackend const&) = delete;
	void operator=(FiberSchedulerBackend const&) = delete;

	template<typename Fn, typename ... Args>
	void AddTask(TaskParameters, Fn &&, Args && ...);


	template<typename Fn, typename ... Args>
	void ForEachThread(TaskPriority, Fn &&, Args && ...);

	void Block() const;
	static void Yield();

	template< typename Clock, typename Duration>
	static void YieldUntil(std::chrono::time_point< Clock, Duration > const&);

private:

	template< typename Fn >
	static void LaunchFiber(TaskParameters&, Fn &&);

	void InitPointers();
	void ThreadRun();

	bool shouldRun_; //flag to help coordinate destruction

	uint fiberCount_;
	boost::fibers::mutex fiberMutex_;
	boost::fibers::condition_variable fiberCondition_; //no spurious wakeups with the boost::fibers version

	boost::fibers::fiber_specific_ptr<uint> blockCount_;
	boost::fibers::fiber_specific_ptr<boost::fibers::mutex> blockMutex_;
	boost::fibers::fiber_specific_ptr<boost::fibers::condition_variable> blockCondition_;  //no spurious wakeups with the boost::fibers version

	Property<uint>& threadCount_;
	std::vector<std::thread> childThreads_;

};

/*
 * Adds a task to the FiberSchedulerBackend to be executed on any available thread.
 *
 * @tparam	Fn  	Type of the function.
 * @tparam	Args	Type of the arguments.
 * @param 		  	params	Options for controlling the operation.
 * @param [in,out]	fn	  	The function.
 * @param 		  	args  	Function Arguments The arguments.
 */

template<typename Fn, typename ... Args>
void FiberSchedulerBackend::AddTask(TaskParameters params, Fn && fn, Args && ... args) {

	//increment the global fiber count
	{
		std::scoped_lock<boost::fibers::mutex> incrementLock(fiberMutex_);
		fiberCount_++;
	}

	auto executeFiber = [this, fn, params, &args...]() mutable {

		//init the data associated with the fiber
		InitPointers();

		/////////////////////////////////////////////
		std::invoke(fn, std::forward<Args>(args)...);
		/////////////////////////////////////////////

		//All children fibers must be completed
		Block();

		//execution finished, decrease the global fiber count
		size_t remainingFibers;
		{
			std::scoped_lock<boost::fibers::mutex> incrementLock(fiberMutex_);
			fiberCount_--;
			remainingFibers = fiberCount_;
		}

		if (remainingFibers == 0) {
			fiberCondition_.notify_all();
		}

	};

	if (params.shouldBlock_) {

		//grab the parent fiber data and the relevant locks
		boost::fibers::mutex* holdLock = blockMutex_.get();
		uint* holdSize = blockCount_.get();
		boost::fibers::condition_variable* holdConditional = blockCondition_.get();

		//increment the parent fiber count
		{
			std::scoped_lock<boost::fibers::mutex> incrementLock(*holdLock);
			(*holdSize)++;
		}

		LaunchFiber(
			params,
			[executeFiber, holdLock, holdSize, holdConditional, &args...]() mutable {

			executeFiber();

			//decrement the parent fiber count and notify the parent if it reached 0
			size_t remainingHolds;
			{
				std::scoped_lock<boost::fibers::mutex> incrementLock(*holdLock);
				(*holdSize)--;
				remainingHolds = *holdSize;
			}

			if (remainingHolds == 0) {
				holdConditional->notify_all();
			}

		});

	}
	else {

		LaunchFiber(
			params,
			executeFiber
		);

	}

}

template<typename Fn, typename ... Args>
void FiberSchedulerBackend::ForEachThread(TaskPriority priority, Fn && fn, Args && ... args) {

	//immediately enter to provide block scope to the perthread tasks
	TaskParameters subParams(false);
	AddTask(subParams, [this, priority, fn, &args...]() {

		TaskParameters usedParams(true, priority);

		for (uint threadIndex = 0; threadIndex < threadCount_; ++threadIndex) {

			usedParams.requiredThread_ = threadIndex;
			AddTask(usedParams, fn, std::forward<Args>(args)...);

		}

	});

}

template< typename Fn >
void FiberSchedulerBackend::LaunchFiber(TaskParameters& params, Fn && func) {

	if (params.post_) {

		boost::fibers::fiber fiber(boost::fibers::launch::post, func);

		fiber.properties<FiberProperties>().SetProperties(params.priority_, params.requiredThread_); //fiber not pushed to shared queues until properties are set
		fiber.detach();

	}
	else {

		//since the fiber is not awoken like a posted fiber, and immediately entered, its property should not be new
		boost::fibers::fiber fiber(boost::fibers::launch::dispatch, func);

		//Don't let fiber fall out of scope (error). If already executed with swap, destruct the context
		fiber.join();

	}

}

template< typename Clock, typename Duration>
void FiberSchedulerBackend::YieldUntil(std::chrono::time_point< Clock, Duration > const& timePoint) {

	boost::fibers::mutex mutex;
	boost::fibers::condition_variable conditional;

	std::unique_lock<boost::fibers::mutex> lock(mutex);
	conditional.wait_until(lock, timePoint);

}