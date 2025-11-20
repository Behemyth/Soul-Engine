export module synodic.soul.engine:core.object;

import std;
import synodic.soul.core;

export class Object {
public:
	Object() = default;

	std::uint32_t verticeAmount;
	std::uint32_t faceAmount;
	std::uint32_t tetAmount;
	std::uint32_t materialAmount;
};
