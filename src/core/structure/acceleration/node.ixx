export module synodic.soul.engine:core.structure.acceleration.node;

import std;
import synodic.soul.core;
import :core.geometry.bounding_box;

export class Node
{

public:

	BoundingBox box;

	std::uint32_t childLeft;
	std::uint32_t rangeLeft;

	std::uint32_t childRight;
	std::uint32_t rangeRight;

	std::uint32_t atomic;

};
