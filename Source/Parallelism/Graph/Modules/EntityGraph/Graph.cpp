#include "Graph.h"

#include "Parallelism/Scheduler/SchedulerModule.h"


Graph::Graph(std::shared_ptr<SchedulerModule>& scheduler):
	scheduler_(scheduler)
{

}

//Returns after dispatching the graph.
//Call scheduler_->Block() to guarantee completion
void Graph::Execute(std::chrono::nanoseconds targetDuration) {

	for (const auto& child : children_) {

		if (child->Root()) {

			child->Execute();

		}

	}

}

//Create a sub-graph
Graph& Graph::CreateGraph()
{

	return graphs_.emplace_front(scheduler_);

}