export module synodic.soul.scheduler:task_parameters;

import std;

export enum class TaskPriority {
	LOW,    // A comparative low priority task.
	HIGH,   // A high priority task.
	UX      // User Experience. Takes precedence over all.
};

export class TaskParameters {
public:
	TaskParameters(bool = true, TaskPriority = TaskPriority::HIGH, bool = true, std::int32_t = -1);
	~TaskParameters() = default;

	TaskParameters(const TaskParameters&) = default;
	TaskParameters(TaskParameters&& o) noexcept = default;

	TaskParameters& operator=(const TaskParameters&) = default;
	TaskParameters& operator=(TaskParameters&& other) noexcept = default;

	bool post_; // the fiber will not execute immediately and instead run when a scheduler picks it up. If not true, the fiber is required to remain on this thread and other parameters dont matter.
	TaskPriority priority_;
	bool shouldBlock_; // attaches the fiber to its parent. Guaranteed to complete before the parent does and can synchronize with `Block()`
	std::int32_t requiredThread_; // forces the fiber to run on a specified thread, any negative number runs on any thread
};

inline TaskParameters::TaskParameters(bool post,
	TaskPriority priority,
	bool shouldBlock,
	std::int32_t requiredThread) :
	post_(post),
	priority_(priority),
	shouldBlock_(shouldBlock),
	requiredThread_(requiredThread) {

	if (!post_) {
		shouldBlock_ = true;
		requiredThread_ = -1; // if not post, ignored in scheduler
		priority_ = TaskPriority::HIGH;
	}
}
