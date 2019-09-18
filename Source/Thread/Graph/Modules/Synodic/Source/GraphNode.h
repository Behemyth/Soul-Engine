#pragma once

#include "Parallelism/Scheduler/TaskParameters.h"

#include <vector>
#include <chrono>

//Handles the topology of the graph
class GraphNode{

public:

	GraphNode();
	virtual ~GraphNode() = default;

	GraphNode(const GraphNode&) = delete;
	GraphNode(GraphNode&&) noexcept = default;

	GraphNode& operator=(const GraphNode&) = delete;
	GraphNode& operator=(GraphNode&&) noexcept = default;

	virtual void Execute(std::chrono::nanoseconds = std::chrono::nanoseconds(0)) = 0;

	void DependsOn(GraphNode&);

	bool Root();
	void Root(bool);

protected:

	TaskParameters parameters_;
	std::vector<GraphNode*> children_;


private:

	void AddChild(GraphNode*);

	bool root;

};
