export module synodic.soul.scheduler.backend.passthrough:scheduler;
import std;

import synodic.soul.core;
import synodic.soul.scheduler;

export class PassthroughSchedulerBackend {

public:

	PassthroughSchedulerBackend(Property<std::uint32_t>&);
	~PassthroughSchedulerBackend() = default;

	PassthroughSchedulerBackend(PassthroughSchedulerBackend const&) = delete;
	void operator=(PassthroughSchedulerBackend const&) = delete;

	template<typename Fn, typename ... Args>
	void AddTask(TaskParameters, Fn &&, Args && ...);

	template<typename Fn, typename ... Args>
	void ForEachThread(TaskPriority, Fn &&, Args && ...);

	void Block() const;
	static void Yield();

	template< typename Clock, typename Duration>
	static void YieldUntil(std::chrono::time_point< Clock, Duration > const&);

private:

	Property<std::uint32_t>& threadCount_;

};

/*
 * Passthrough scheduler backend constructor.
 * Stores the thread count property for compatibility but doesn't use it (single-threaded).
 *
 * @param threadCount The thread count property reference.
 */
inline PassthroughSchedulerBackend::PassthroughSchedulerBackend(Property<std::uint32_t>& threadCount) :
	threadCount_(threadCount) {
}

/*
 * Adds a task to be executed immediately on the calling thread.
 * All TaskParameters are ignored - execution is always synchronous.
 *
 * @tparam	Fn  	Type of the function.
 * @tparam	Args	Type of the arguments.
 * @param 		  	params	Options for controlling the operation (ignored).
 * @param [in,out]	fn	  	The function to execute.
 * @param 		  	args  	Function arguments.
 */
template<typename Fn, typename ... Args>
void PassthroughSchedulerBackend::AddTask(TaskParameters params, Fn && fn, Args && ... args) {
	// Immediate execution on calling thread, ignoring all parameters
	std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...);
}

/*
 * Executes a task once on the main thread (single-threaded passthrough).
 * In a multi-threaded scheduler, this would run once per thread.
 *
 * @tparam	Fn  	Type of the function.
 * @tparam	Args	Type of the arguments.
 * @param 		  	priority	Priority hint (ignored in passthrough).
 * @param [in,out]	fn	  		The function to execute.
 * @param 		  	args  		Function arguments.
 */
template<typename Fn, typename ... Args>
void PassthroughSchedulerBackend::ForEachThread(TaskPriority priority, Fn && fn, Args && ... args) {
	// Single-threaded: execute once on the calling thread
	std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...);
}

/*
 * Blocks until all child tasks complete.
 * In passthrough mode, this is a no-op since all tasks execute synchronously.
 */
inline void PassthroughSchedulerBackend::Block() const {
	// No-op: all tasks already completed synchronously
}

/*
 * Yields execution to allow other tasks to run.
 * In passthrough mode, this hints to the OS scheduler.
 */
inline void PassthroughSchedulerBackend::Yield() {
	std::this_thread::yield();
}

/*
 * Yields until a specific time point.
 * Suspends the calling thread until the specified time.
 *
 * @tparam Clock	The clock type.
 * @tparam Duration	The duration type.
 * @param timePoint	The time point to wait until.
 */
template< typename Clock, typename Duration>
void PassthroughSchedulerBackend::YieldUntil(std::chrono::time_point< Clock, Duration > const& timePoint) {
	std::this_thread::sleep_until(timePoint);
}
