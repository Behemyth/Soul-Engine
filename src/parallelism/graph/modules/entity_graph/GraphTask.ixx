export module synodic.soul.graph:graph_task;

import :graph_node;
import synodic.soul.scheduler;
import std;

export template<typename SchedulerType> requires SchedulerBackend<SchedulerType>
class GraphTask : public GraphNode {
public:
	GraphTask(SchedulerType& scheduler) noexcept:
		scheduler_(scheduler)
	{
	}

	GraphTask(SchedulerType& scheduler, std::function<void()>&& callable) noexcept:
		scheduler_(scheduler),
		callable_(std::forward<std::function<void()>>(callable))
	{
	}

	~GraphTask() override = default;

	GraphTask(const GraphTask&) = delete;
	GraphTask(GraphTask&&) = default;

	GraphTask& operator=(const GraphTask&) = delete;
	GraphTask& operator=(GraphTask&&) = default;

	void Execute(std::chrono::nanoseconds targetDuration) override {
		scheduler_.AddTask(parameters_, [this]()
		{
			std::invoke(std::forward<std::function<void()>>(callable_));

			for (const auto& child : children_) {
				child->Execute();
			}

			scheduler_.Block();
		});
	}

private:
	SchedulerType& scheduler_;
	std::function<void()> callable_;
};
