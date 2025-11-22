export module synodic.soul.scheduler;

export import :task_parameters;

import synodic.soul.core;
import :task_parameters;
import std;

export template<typename T>
concept SchedulerBackend = requires(T scheduler, TaskParameters params, TaskPriority priority) {
	{ scheduler.AddTask(params, std::declval<std::function<void()>>()) } -> std::same_as<void>;
	{ scheduler.ForEachThread(priority, std::declval<std::function<void()>>()) } -> std::same_as<void>;
	{ scheduler.Block() } -> std::same_as<void>;
	{ scheduler.Yield() } -> std::same_as<void>;
	{ scheduler.YieldUntil(std::declval<std::chrono::steady_clock::time_point>()) } -> std::same_as<void>;
};
