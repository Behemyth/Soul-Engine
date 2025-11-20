export module synodic.soul.tracer:film;

import std;

// TODO: Replace with proper vector types
struct uvec2 { unsigned int x, y; };
struct vec4 { float x, y, z, w; };

export class Film {
public:
	Film();
	~Film();

	uvec2 resolutionPrev;
	uvec2 resolution;
	uvec2 resolutionMax;

	float resolutionRatio;

	vec4* results;
	std::int32_t* hits;
};
