export module synodic.soul.scheduler:scheduler_module;

import synodic.soul.core;
import :task_parameters;
import std;

// Temporary placeholder scheduler backends until legacy headers are converted to modules
// These will be replaced with proper implementations once Boost.Fiber is integrated
#if defined(FIBER_SCHEDULER)
struct FiberSchedulerBackend {
	FiberSchedulerBackend(Property<std::uint32_t>&) {}
	template<typename... Args> void AddTask(Args&&...) {}
	template<typename... Args> void ForEachThread(Args&&...) {}
	template<typename... Args> void YieldUntil(Args&&...) {}
	void Yield() {}
	void Block() const {}
};
#else
struct DispatchSchedulerBackend {
	DispatchSchedulerBackend(Property<std::uint32_t>&) {}
	template<typename... Args> void AddTask(Args&&...) {}
	template<typename... Args> void ForEachThread(Args&&...) {}
	template<typename... Args> void YieldUntil(Args&&...) {}
	void Yield() {}
	void Block() const {}
};
#endif

export enum class SchedulerID { Fiber, Dispatch };

export class SchedulerModule : public Module<SchedulerModule> {
public:
	SchedulerModule(Property<std::uint32_t>&);
	virtual ~SchedulerModule() = default;

	SchedulerModule(const SchedulerModule&) = delete;
	SchedulerModule(SchedulerModule&&) noexcept = default;

	SchedulerModule& operator=(const SchedulerModule&) = delete;
	SchedulerModule& operator=(SchedulerModule&&) noexcept = default;

	template<typename Fn, typename... Args>
	void AddTask(TaskParameters, Fn&&, Args&&...);

	template<typename Fn, typename... Args>
	void ForEachThread(TaskPriority, Fn&&, Args&&...);

	void Block() const;
	void Yield();

	template<typename Clock, typename Duration>
	void YieldUntil(std::chrono::time_point<Clock, Duration> const&);

	// Factory
	static std::shared_ptr<SchedulerModule> CreateModule(Property<std::uint32_t>&);

private:
	#if defined(FIBER_SCHEDULER)
	constexpr static SchedulerID schedulerID_ = SchedulerID::Fiber;
	FiberSchedulerBackend scheduler_;
	#else
	constexpr static SchedulerID schedulerID_ = SchedulerID::Dispatch;
	DispatchSchedulerBackend scheduler_;
	#endif
};

// Implementation
inline SchedulerModule::SchedulerModule(Property<std::uint32_t>& threadCount) :
	scheduler_(threadCount) {
}

inline void SchedulerModule::Block() const {
	scheduler_.Block();
}

inline void SchedulerModule::Yield() {
	scheduler_.Yield();
}

inline std::shared_ptr<SchedulerModule> SchedulerModule::CreateModule(Property<std::uint32_t>& threadCount) {
	return std::make_shared<SchedulerModule>(threadCount);
}

// Template implementations
template<typename Fn, typename... Args>
void SchedulerModule::ForEachThread(TaskPriority priority, Fn&& fn, Args&&... args) {
	scheduler_.ForEachThread(priority, fn, std::forward<Args>(args)...);
}

template<typename Fn, typename... Args>
void SchedulerModule::AddTask(TaskParameters params, Fn&& fn, Args&&... args) {
	scheduler_.AddTask(params, fn, std::forward<Args>(args)...);
}

template<typename Clock, typename Duration>
void SchedulerModule::YieldUntil(std::chrono::time_point<Clock, Duration> const& timePoint) {
	scheduler_.YieldUntil(timePoint);
}
