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
////	__global__ void ExecuteJobs(std::uint32_t n, Ray* rays, BVH* bvh, Vertex* vertices, Face* faces, BoundingBox*, std::int32_t* counter);
////	__global__ void ProcessHits(std::uint32_t n, RayJob* job, std::int32_t jobSize, Ray* rays, Ray* raysNew, Sky* sky, Face* faces, Vertex* vertices, Material* materials, std::int32_t * nAtomic, curandState* randomState);
////	__global__ void EngineSetup(std::uint32_t n, RayJob* jobs, std::int32_t jobSize);
////	__global__ void RaySetup(std::uint32_t n, std::uint32_t jobSize, RayJob* job, Ray* rays, std::int32_t* nAtomic, curandState* randomState);
////	__global__ void RandomSetup(std::uint32_t n, curandState* randomState, std::uint32_t raySeed);
////
////}
