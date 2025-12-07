export module synodic.soul.tracer:ray;

import std;
import synodic.library;
import synodic.soul.core;

export class Ray {
public:
	synodic::math::vec4 storage;
	synodic::math::vec4 origin;
	synodic::math::vec4 direction;
	synodic::math::vec2 bary;
	std::uint32_t currentHit;
	std::uint32_t resultOffset;
	char job;
};
