#pragma once

//#include "Tracer/RayJob.h"
//#include "Core/Scene/Scene.h"
//#include "Tracer/Ray.h"
//#include <curand_kernel.h>
//
////class BVH;
////
////namespace RayEngineCUDA {
////
////	__global__ void ExecuteJobs(uint n, Ray* rays, BVH* bvh, Vertex* vertices, Face* faces, BoundingBox*, int* counter);
////	__global__ void ProcessHits(uint n, RayJob* job, int jobSize, Ray* rays, Ray* raysNew, Sky* sky, Face* faces, Vertex* vertices, Material* materials, int * nAtomic, curandState* randomState);
////	__global__ void EngineSetup(uint n, RayJob* jobs, int jobSize);
////	__global__ void RaySetup(uint n, uint jobSize, RayJob* job, Ray* rays, int* nAtomic, curandState* randomState);
////	__global__ void RandomSetup(uint n, curandState* randomState, uint raySeed);
////
////}
