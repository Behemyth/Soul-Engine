#pragma once

#include "GraphNode.h"
#include "GraphTask.h"

#include "Core/Utility/Template/TypeTraits.h"

#include <forward_list>

class SchedulerModule;

class Graph : public GraphNode {


public:

	Graph(std::shared_ptr<SchedulerModule>&);
	~Graph() override = default;

	Graph(const Graph&) = delete;
	Graph(Graph&&) noexcept = default;

	Graph& operator=(const Graph&) = delete;
	Graph& operator=(Graph&&) noexcept = default;

	template <typename Callable>
	GraphTask& AddTask(Callable&&);
	Graph& CreateGraph();
	
	void Execute(std::chrono::nanoseconds) override;


private:

	std::shared_ptr<SchedulerModule> scheduler_;

	std::forward_list<GraphTask> tasks_;
	std::forward_list<Graph> graphs_;


};

//Create a tasks under this graph's control
template <typename Callable>
GraphTask& Graph::AddTask(Callable&& callable)
{

	if constexpr (!std::is_invocable_v<Callable>) {

		static_assert(dependent_false_v<Callable>, "The provided parameter is not callable");

	}

	GraphTask& task = tasks_.emplace_front(scheduler_, std::forward<Callable>(callable));

	task.DependsOn(*this);
	task.Root(true);

	return task;

}