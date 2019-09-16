#include "GraphNode.h"

GraphNode::GraphNode():
	root(false)
{	
}

void GraphNode::DependsOn(GraphNode& other) {

	root = false;
	other.AddChild(this);

}

bool GraphNode::Root() {

	return root;

}

void GraphNode::Root(bool other) {

	root = other;

}

void GraphNode::AddChild(GraphNode* child) {

	children_.push_back(child);

}