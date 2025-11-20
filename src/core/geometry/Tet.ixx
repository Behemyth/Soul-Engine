export module synodic.soul.engine:core.geometry.tet;

import std;
import synodic.soul.core;

// TODO: Replace with proper vector types (glm removed)

export class Tet : Component
{

public:

	Tet() = default;
	~Tet() = default;

	// TODO: Replace with proper vector types (glm removed)
	struct { std::uint32_t x, y, z, w; } indices;
	std::uint32_t material;
	std::uint32_t object;

};
