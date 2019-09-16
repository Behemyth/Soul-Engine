#pragma once

#include "Core/Geometry/BoundingBox.h"
#include "Types.h"

class Node
{

public:

	BoundingBox box;

	uint childLeft;
	uint rangeLeft;

	uint childRight;
	uint rangeRight;

	uint atomic;

};