export module synodic.soul.engine:core.geometry.tet;

import std;
import synodic.library;
import synodic.soul.core;

export class Tet : Component
{

public:

	Tet() = default;
	~Tet() = default;

	synodic::math::uvec4 indices;
	std::uint32_t material;
	std::uint32_t object;

};
