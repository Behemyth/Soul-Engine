module synodic.soul.tracer;

import std;
import synodic.soul.core;
import :camera;

static std::uint32_t counter = 0;

RayJob::RayJob(rayType whatToGet, bool _canChange, float newSamples)
{
	type = whatToGet;
	samples = newSamples;
	rayOffset = 0;
	canChange = _canChange;
	id = counter++;
}
