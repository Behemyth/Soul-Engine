export module synodic.soul.graph:graph;

import synodic.soul.scheduler;
import :graph_node;
import :graph_task;
import std;

export template<SchedulerBackend SchedulerType>
class TaskGraph : public GraphNode {
public:
	explicit TaskGraph(SchedulerType& scheduler) :
		scheduler_(scheduler) {
	}

	~TaskGraph() override = default;

	TaskGraph(const TaskGraph&) = delete;
	TaskGraph(TaskGraph&&) noexcept = default;

	TaskGraph& operator=(const TaskGraph&) = delete;
	TaskGraph& operator=(TaskGraph&&) noexcept = default;

	template <typename Callable>
	GraphTask<SchedulerType>& AddTask(Callable&& callable) {
		if constexpr (!std::is_invocable_v<Callable>) {
			static_assert(std::is_invocable_v<Callable>, "The provided parameter is not callable");
		}

		GraphTask<SchedulerType>& task = tasks_.emplace_front(scheduler_, std::forward<Callable>(callable));

		task.DependsOn(*this);
		task.Root(true);

		return task;
	}

	TaskGraph& CreateGraph() {
		return graphs_.emplace_front(scheduler_);
	}

	void Execute(std::chrono::nanoseconds) override {
		// Execute all root tasks
		for (auto& task : tasks_) {
			if (task.Root()) {
				task.Execute(std::chrono::nanoseconds(0));
			}
		}
	}

private:
	SchedulerType& scheduler_;
	std::forward_list<GraphTask<SchedulerType>> tasks_;
	std::forward_list<TaskGraph<SchedulerType>> graphs_;
};

// Backward compatibility alias
export template<SchedulerBackend SchedulerType>
using Graph = TaskGraph<SchedulerType>;
