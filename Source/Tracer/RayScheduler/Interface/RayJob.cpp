#include "RayJob.h"

static uint counter = 0;

RayJob::RayJob(rayType whatToGet, bool _canChange, float newSamples)
{

	type = whatToGet;
	samples = newSamples;
	rayOffset = 0;
	canChange = _canChange;
	id = counter++;
}