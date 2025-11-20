export module synodic.soul.graph:graph_task;

import synodic.soul.scheduler;
import :graph_node;
import std;

export class GraphTask : public GraphNode {
public:
	GraphTask(std::shared_ptr<SchedulerModule>&) noexcept;
	GraphTask(std::shared_ptr<SchedulerModule>&, std::function<void()>&&) noexcept;

	~GraphTask() override = default;

	GraphTask(const GraphTask&) = delete;
	GraphTask(GraphTask&&) = default;

	GraphTask& operator=(const GraphTask&) = delete;
	GraphTask& operator=(GraphTask&&) = default;

	void Execute(std::chrono::nanoseconds) override;

private:
	std::shared_ptr<SchedulerModule> scheduler_;
	std::function<void()> callable_;
};
