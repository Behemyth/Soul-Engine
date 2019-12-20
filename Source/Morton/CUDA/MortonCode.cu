#include "MortonCode.cuh"
//#include "Parallelism/Compute/CUDA/Utility/CUDAHelper.cuh"
//#include "glm/glm.hpp"
//#include "Parallelism/Compute/DeviceAPI.h"
//
//
//#define TwoE20 1048575 //2^20-1
//#define TwoE21 2097151 //2^21-1
//
//__host__ __device__ uint64 Split64_3D(unsigned int a) {
//	uint64 x = a;
//	x = (x | x << 32) & 0x1f00000000ffff;
//	x = (x | x << 16) & 0x1f0000ff0000ff;
//	x = (x | x << 8) & 0x100f00f00f00f00f;
//	x = (x | x << 4) & 0x10c30c30c30c30c3;
//	x = (x | x << 2) & 0x1249249249249249;
//	return x;
//}
//
//__host__ __device__ uint Unsplit64_3D(const uint64 m) {
//	uint64 x = m & 0x1249249249249249;
//	x = (x ^ (x >> 2)) & 0x10c30c30c30c30c3;
//	x = (x ^ (x >> 4)) & 0x100f00f00f00f00f;
//	x = (x ^ (x >> 8)) & 0x1f0000ff0000ff;
//	x = (x ^ (x >> 16)) & 0x1f00000000ffff;
//	x = (x ^ (x >> 32)) & 0x1fffff;
//	return static_cast<uint>(x);
//}
//
//__host__ __device__ uint64 Split64_2D(unsigned int a) {
//	uint64 x = a;
//	x = (x | x << 32) & 0x00000000FFFFFFFF;
//	x = (x | x << 16) & 0x0000FFFF0000FFFF;
//	x = (x | x << 8) & 0x00FF00FF00FF00FF;
//	x = (x | x << 4) & 0x0F0F0F0F0F0F0F0F;
//	x = (x | x << 2) & 0x3333333333333333;
//	x = (x | x << 1) & 0x5555555555555555;
//
//	return x;
//}
//
//__host__ __device__ uint Unsplit64_2D(const uint64 m) {
//	uint64 x = m & 0x3333333333333333;
//	x = (x ^ (x >> 2)) & 0x0F0F0F0F0F0F0F0F;
//	x = (x ^ (x >> 4)) & 0x00FF00FF00FF00FF;
//	x = (x ^ (x >> 8)) & 0x0000FFFF0000FFFF;
//	x = (x ^ (x >> 16)) & 0x00000000FFFFFFFF;
//	return static_cast<uint>(x);
//}
//
//__host__ __device__ uint64 MortonCode::Calculate64_3D(const glm::vec3& data) {
//
//	//2^20 mapped to [-1,1] and then 2^21 [0,1]
//	uint x = int(data.x*TwoE20) + TwoE20;
//	uint y = int(data.y*TwoE20) + TwoE20;
//	uint z = int(data.z*TwoE20) + TwoE20;
//
//	uint64 answer = 0;
//	answer |= Split64_3D(x) | Split64_3D(y) << 1 | Split64_3D(z) << 2;
//	return answer;
//}
//
//__host__ __device__ glm::vec3 MortonCode::Decode64_3D(uint64 m) {
//
//	uint x = Unsplit64_3D(m);
//	uint y = Unsplit64_3D(m >> 1);
//	uint z = Unsplit64_3D(m >> 2);
//
//	glm::vec3 data = glm::vec3(x / TwoE20, y / TwoE20, z / TwoE20) - 1.0f;
//
//	return data;
//}
//
//__host__ __device__ uint64 MortonCode::Calculate64_2D(const glm::vec2& data) {
//
//	//2^20 mapped to [-1,1] and then 2^21 [0,1]
//	uint x = int(data.x*TwoE20) + TwoE20;
//	uint y = int(data.y*TwoE20) + TwoE20;
//
//	uint64 answer = 0;
//	answer |= Split64_2D(x) | Split64_2D(y) << 1;
//	return answer;
//}
//
//__host__ __device__ glm::vec2 MortonCode::Decode64_2D(uint64 m) {
//
//	uint x = Unsplit64_2D(m);
//	uint y = Unsplit64_2D(m >> 1);
//
//	glm::vec2 data = glm::vec2(x / TwoE20, y / TwoE20) - 1.0f;
//
//	return data;
//}
//
////Works with unsigned vectors
//
//__host__ __device__ uint64 MortonCode::Calculate64_3D(const glm::uvec3& data) {
//
//	uint64 answer = 0;
//	answer |= Split64_3D(data.x) | Split64_3D(data.y) << 1 | Split64_3D(data.z) << 2;
//	return answer;
//}
//
//__host__ __device__ glm::uvec3 MortonCode::Decode64U_3D(uint64 m) {
//
//	uint x = Unsplit64_3D(m);
//	uint y = Unsplit64_3D(m >> 1);
//	uint z = Unsplit64_3D(m >> 2);
//
//	glm::uvec3 data = glm::uvec3(x, y, z);
//
//	return data;
//}
//
//__host__ __device__ uint64 MortonCode::Calculate64_2D(const glm::uvec2& data) {
//
//	uint64 answer = 0;
//	answer |= Split64_2D(data.x) | Split64_2D(data.y) << 1;
//	return answer;
//}
//
//__host__ __device__ glm::uvec2 MortonCode::Decode64U_2D(uint64 m) {
//
//	uint x = Unsplit64_2D(m);
//	uint y = Unsplit64_2D(m >> 1);
//
//	glm::vec2 data = glm::vec2(x, y);
//
//	return data;
//}
//
////TODO split into two kernals for Scene.cu
//__global__ void MortonCode::ComputeGPUFace64(uint n, uint64* mortonCodes, Face* faces, Vertex* vertices) {
//
//	const uint index = ThreadIndex1D();
//
//	if (index >= n) {
//		return;
//	}
//
//	glm::uvec3 ind = faces[index].indices;
//	glm::vec3 centroid = ((vertices + ind.x)->position + (vertices + ind.y)->position + (vertices + ind.z)->position) / 3.0f;
//
//	mortonCodes[index] = Calculate64_3D(centroid);
//}
//
//__global__ void MortonCode::ComputeGPU64(uint n, uint64* mortonCodes, glm::uvec2* data) {
//
//	uint index = ThreadIndex1D();
//
//	if (index >= n) {
//		return;
//	}
//
//	mortonCodes[index] = Calculate64_2D(data[index]);
//}