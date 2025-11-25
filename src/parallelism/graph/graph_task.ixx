export module synodic.soul.graph:graph_task;

import synodic.soul.scheduler;
import :graph_node;
import std;

export template<SchedulerBackend SchedulerType>
class GraphTask : public GraphNode {
public:
	GraphTask(SchedulerType& scheduler) noexcept :
		scheduler_(scheduler) {
	}

	GraphTask(SchedulerType& scheduler, std::function<void()>&& callable) noexcept :
		scheduler_(scheduler),
		callable_(std::move(callable)) {
	}

	~GraphTask() override = default;

	GraphTask(const GraphTask&) = delete;
	GraphTask(GraphTask&&) = default;

	GraphTask& operator=(const GraphTask&) = delete;
	GraphTask& operator=(GraphTask&&) = default;

	void Execute(std::chrono::nanoseconds) override {
		if (callable_) {
			callable_();
		}
	}

private:
	SchedulerType& scheduler_;
	std::function<void()> callable_;
};
