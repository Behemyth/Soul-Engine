#include "MortonCode.cuh"
//#include "Parallelism/Compute/CUDA/Utility/CUDAHelper.cuh"
//#include "glm/glm.hpp"
//#include "Parallelism/Compute/DeviceAPI.h"
//
//
//#define TwoE20 1048575 //2^20-1
//#define TwoE21 2097151 //2^21-1
//
//__host__ __device__ std::int64_t Split64_3D(std::uint32_t a) {
//	std::int64_t x = a;
//	x = (x | x << 32) & 0x1f00000000ffff;
//	x = (x | x << 16) & 0x1f0000ff0000ff;
//	x = (x | x << 8) & 0x100f00f00f00f00f;
//	x = (x | x << 4) & 0x10c30c30c30c30c3;
//	x = (x | x << 2) & 0x1249249249249249;
//	return x;
//}
//
//__host__ __device__ std::uint32_t Unsplit64_3D(const std::int64_t m) {
//	std::int64_t x = m & 0x1249249249249249;
//	x = (x ^ (x >> 2)) & 0x10c30c30c30c30c3;
//	x = (x ^ (x >> 4)) & 0x100f00f00f00f00f;
//	x = (x ^ (x >> 8)) & 0x1f0000ff0000ff;
//	x = (x ^ (x >> 16)) & 0x1f00000000ffff;
//	x = (x ^ (x >> 32)) & 0x1fffff;
//	return static_cast<std::uint32_t>(x);
//}
//
//__host__ __device__ std::int64_t Split64_2D(std::uint32_t a) {
//	std::int64_t x = a;
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
//__host__ __device__ std::uint32_t Unsplit64_2D(const std::int64_t m) {
//	std::int64_t x = m & 0x3333333333333333;
//	x = (x ^ (x >> 2)) & 0x0F0F0F0F0F0F0F0F;
//	x = (x ^ (x >> 4)) & 0x00FF00FF00FF00FF;
//	x = (x ^ (x >> 8)) & 0x0000FFFF0000FFFF;
//	x = (x ^ (x >> 16)) & 0x00000000FFFFFFFF;
//	return static_cast<std::uint32_t>(x);
//}
//
//__host__ __device__ std::int64_t MortonCode::Calculate64_3D(const glm::vec3& data) {
//
//	//2^20 mapped to [-1,1] and then 2^21 [0,1]
//	std::uint32_t x = std::int32_t(data.x*TwoE20) + TwoE20;
//	std::uint32_t y = std::int32_t(data.y*TwoE20) + TwoE20;
//	std::uint32_t z = std::int32_t(data.z*TwoE20) + TwoE20;
//
//	std::int64_t answer = 0;
//	answer |= Split64_3D(x) | Split64_3D(y) << 1 | Split64_3D(z) << 2;
//	return answer;
//}
//
//__host__ __device__ glm::vec3 MortonCode::Decode64_3D(std::int64_t m) {
//
//	std::uint32_t x = Unsplit64_3D(m);
//	std::uint32_t y = Unsplit64_3D(m >> 1);
//	std::uint32_t z = Unsplit64_3D(m >> 2);
//
//	glm::vec3 data = glm::vec3(x / TwoE20, y / TwoE20, z / TwoE20) - 1.0f;
//
//	return data;
//}
//
//__host__ __device__ std::int64_t MortonCode::Calculate64_2D(const glm::vec2& data) {
//
//	//2^20 mapped to [-1,1] and then 2^21 [0,1]
//	std::uint32_t x = std::int32_t(data.x*TwoE20) + TwoE20;
//	std::uint32_t y = std::int32_t(data.y*TwoE20) + TwoE20;
//
//	std::int64_t answer = 0;
//	answer |= Split64_2D(x) | Split64_2D(y) << 1;
//	return answer;
//}
//
//__host__ __device__ glm::vec2 MortonCode::Decode64_2D(std::int64_t m) {
//
//	std::uint32_t x = Unsplit64_2D(m);
//	std::uint32_t y = Unsplit64_2D(m >> 1);
//
//	glm::vec2 data = glm::vec2(x / TwoE20, y / TwoE20) - 1.0f;
//
//	return data;
//}
//
////Works with unsigned vectors
//
//__host__ __device__ std::int64_t MortonCode::Calculate64_3D(const glm::uvec3& data) {
//
//	std::int64_t answer = 0;
//	answer |= Split64_3D(data.x) | Split64_3D(data.y) << 1 | Split64_3D(data.z) << 2;
//	return answer;
//}
//
//__host__ __device__ glm::uvec3 MortonCode::Decode64U_3D(std::int64_t m) {
//
//	std::uint32_t x = Unsplit64_3D(m);
//	std::uint32_t y = Unsplit64_3D(m >> 1);
//	std::uint32_t z = Unsplit64_3D(m >> 2);
//
//	glm::uvec3 data = glm::uvec3(x, y, z);
//
//	return data;
//}
//
//__host__ __device__ std::int64_t MortonCode::Calculate64_2D(const glm::uvec2& data) {
//
//	std::int64_t answer = 0;
//	answer |= Split64_2D(data.x) | Split64_2D(data.y) << 1;
//	return answer;
//}
//
//__host__ __device__ glm::uvec2 MortonCode::Decode64U_2D(std::int64_t m) {
//
//	std::uint32_t x = Unsplit64_2D(m);
//	std::uint32_t y = Unsplit64_2D(m >> 1);
//
//	glm::vec2 data = glm::vec2(x, y);
//
//	return data;
//}
//
////TODO split into two kernals for Scene.cu
//__global__ void MortonCode::ComputeGPUFace64(std::uint32_t n, std::int64_t* mortonCodes, Face* faces, Vertex* vertices) {
//
//	const std::uint32_t index = ThreadIndex1D();
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
//__global__ void MortonCode::ComputeGPU64(std::uint32_t n, std::int64_t* mortonCodes, glm::uvec2* data) {
//
//	std::uint32_t index = ThreadIndex1D();
//
//	if (index >= n) {
//		return;
//	}
//
//	mortonCodes[index] = Calculate64_2D(data[index]);
//}
