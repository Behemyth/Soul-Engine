export module synodic.soul.tracer:ray;

import std;
import synodic.periapsis;
import synodic.soul.core;

export class Ray {
public:
	peri::math::vec4 storage;
	peri::math::vec4 origin;
	peri::math::vec4 direction;
	peri::math::vec2 bary;
	std::uint32_t currentHit;
	std::uint32_t resultOffset;
	char job;
};
