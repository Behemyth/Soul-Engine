export module synodic.soul.engine:core.geometry.bounding_box;

import std;
import synodic.soul.core;

// TODO: Replace with proper vector types (glm removed)

export class BoundingBox : Component
{

public:

	BoundingBox() = default;
	~BoundingBox() = default;

	// TODO: Replace with proper vector types (glm removed)
	struct { float x, y, z; } min;
	struct { float x, y, z; } max;

};
