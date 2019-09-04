#pragma once

#include "Core/Interface/Module/Module.h"

#include "Types.h"
#include "Property.h"
#include "TaskParameters.h"

#include <chrono>
#include <memory>

#if defined(FIBER_SCHEDULER)

#include "Modules/Fiber/FiberSchedulerBackend.h"

#else

#include "Modules/Dispatch/DispatchSchedulerBackend.h"

#endif

enum class SchedulerID { Fiber, Dispatch };

class SchedulerModule : public Module<SchedulerModule> {

public:

	SchedulerModule(Property<uint>&);
	virtual ~SchedulerModule() = default;

	SchedulerModule(const SchedulerModule &) = delete;
	SchedulerModule(SchedulerModule &&) noexcept = default;

	SchedulerModule& operator=(const SchedulerModule &) = delete;
	SchedulerModule& operator=(SchedulerModule &&) noexcept = default;

	template<typename Fn, typename... Args>
	void AddTask(TaskParameters, Fn&&, Args&&...);


	template<typename Fn, typename... Args>
	void ForEachThread(TaskPriority, Fn&&, Args&&...);

	void Block() const;
	void Yield();

	template<typename Clock, typename Duration>
	void YieldUntil(std::chrono::time_point<Clock, Duration> const&);


	//Factory
	static std::shared_ptr<SchedulerModule> CreateModule(Property<uint>&);


private:

	#if defined(FIBER_SCHEDULER)

	constexpr static SchedulerID schedulerID_ = SchedulerID::Fiber;
	FiberSchedulerBackend scheduler_;

	#else

	constexpr static SchedulerID schedulerID_ = SchedulerID::Dispatch;
	DispatchSchedulerBackend scheduler_;

	#endif

};


template<typename Fn, typename... Args>
void SchedulerModule::ForEachThread(TaskPriority priority, Fn&& fn, Args&&... args)
{

	scheduler_.ForEachThread(priority, fn, std::forward<Args>(args)...);

}

template<typename Fn, typename... Args>
void SchedulerModule::AddTask(TaskParameters params, Fn&& fn, Args&&... args)
{

	scheduler_.AddTask(params, fn, std::forward<Args>(args)...);


}

template<typename Clock, typename Duration>
void SchedulerModule::YieldUntil(std::chrono::time_point<Clock, Duration> const& timePoint)
{

	scheduler_.YieldUntil(timePoint);

}