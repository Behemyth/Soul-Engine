export module synodic.soul.tracer:film;

import std;
import synodic.periapsis;

export class Film {
public:
	Film();
	~Film();

	peri::math::uvec2 resolutionPrev;
	peri::math::uvec2 resolution;
	peri::math::uvec2 resolutionMax;

	float resolutionRatio;

	peri::math::vec4* results;
	std::int32_t* hits;
};
