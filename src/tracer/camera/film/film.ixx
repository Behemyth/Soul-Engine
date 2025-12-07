export module synodic.soul.tracer:film;

import std;
import synodic.library;

export class Film {
public:
	Film();
	~Film();

	synodic::math::uvec2 resolutionPrev;
	synodic::math::uvec2 resolution;
	synodic::math::uvec2 resolutionMax;

	float resolutionRatio;

	synodic::math::vec4* results;
	std::int32_t* hits;
};
