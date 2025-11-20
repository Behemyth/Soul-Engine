export module synodic.soul.graph:graph_node;

import synodic.soul.scheduler;
import std;

// Handles the topology of the graph
export class GraphNode {
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
