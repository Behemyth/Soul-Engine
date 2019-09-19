#pragma once

//#include "Types.h"
//#include "Core/Scene/Bounding Volume Heirarchy/LBVHManager.h"
//
//#include "Core/Geometry/Face.h"
//#include "Core/Geometry/Vertex.h"

//namespace MortonCode{
//
//
//	__global__ void ComputeGPUFace64(uint n, uint64* mortonCodes, Face* faces, Vertex* vertices);
//
//	__global__ void ComputeGPU64(uint n, uint64* mortonCodes, glm::uvec2* data);
//
//
//	//given a point in space with the range [-1,1] for each dimension
//	__host__ __device__ uint64 Calculate64_3D(const glm::vec3&);
//
//	//given a point in space with the range [-1,1] for each dimension
//	__host__ __device__ uint64 Calculate64_2D(const glm::vec2&);
//
//	//returns a point in space with the range [-1,1] for each dimension 
//	__host__ __device__ glm::vec3 Decode64_3D(const uint64);
//
//	//returns a point in space with the range [-1,1] for each dimension 
//	__host__ __device__ glm::vec2 Decode64_2D(const uint64);
//
//	//given a point in space with the range [0,UINT_MAX] for each dimension NOTICE: only first 21 bits used
//	__host__ __device__ uint64 Calculate64_3D(const glm::uvec3&);
//
//	//given a point in space with the range [0,UINT_MAX] for each dimension
//	__host__ __device__ uint64 Calculate64_2D(const glm::uvec2&);
//
//	//returns a point in space with the range [0,UINT_MAX] for each dimension NOTICE: only first 21 bits used
//	__host__ __device__ glm::uvec3 Decode64U_3D(const uint64);
//
//	//returns a point in space with the range [0,UINT_MAX] for each dimension
//	__host__ __device__ glm::uvec2 Decode64U_2D(const uint64);
//}