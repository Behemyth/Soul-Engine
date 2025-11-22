export module synodic.soul.graph:graph;

import :graph_node;
import :graph_task;
import synodic.soul.scheduler;
import std;

export template<typename SchedulerType> requires SchedulerBackend<SchedulerType>
class Graph : public GraphNode {
public:
	Graph(SchedulerType& scheduler):
		scheduler_(scheduler)
	{
	}

	~Graph() override = default;

	Graph(const Graph&) = delete;
	Graph(Graph&&) noexcept = default;

	Graph& operator=(const Graph&) = delete;
	Graph& operator=(Graph&&) noexcept = default;

	template <typename Callable>
	GraphTask<SchedulerType>& AddTask(Callable&& callable) {
		if constexpr (!std::is_invocable_v<Callable>) {
			static_assert(std::false_type::value, "The provided parameter is not callable");
		}

		GraphTask<SchedulerType>& task = tasks_.emplace_front(scheduler_, std::forward<Callable>(callable));

		task.DependsOn(*this);
		task.Root(true);

		return task;
	}

	Graph& CreateGraph()
	{
		return graphs_.emplace_front(scheduler_);
	}

	void Execute(std::chrono::nanoseconds targetDuration) override {
		for (const auto& child : children_) {
			if (child->Root()) {
				child->Execute();
			}
		}
	}

private:
	SchedulerType& scheduler_;
	std::forward_list<GraphTask<SchedulerType>> tasks_;
	std::forward_list<Graph<SchedulerType>> graphs_;
};
