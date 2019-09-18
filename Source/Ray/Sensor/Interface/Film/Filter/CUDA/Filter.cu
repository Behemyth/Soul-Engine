#include "Filter.cuh"

//#include "Parallelism/Compute/DeviceAPI.h"
//
//inline __device__ __host__ uint PowTwoDivider(uint n)
//{
//	if (n == 0) return 0;
//	uint divider = 1;
//	while ((n & divider) == 0) divider <<= 1;
//	return divider;
//}
//
//// t is a value that goes from 0 to 1 to interpolate in a C1 continuous way across uniformly sampled data points.
//// when t is 0, this will return B.  When t is 1, this will return C.  Inbetween values will return an interpolation
//// between B and C.  A and B are used to calculate slopes at the edges.
//__device__ float CubicHermite(float A, float B, float C, float D, float t)
//{
//	float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
//	float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
//	float c = -A / 2.0f + C / 2.0f;
//	float d = B;
//
//	return a*t*t*t + b*t*t + c*t + d;
//}
//
//__device__ const glm::vec4 GetClamped(const glm::vec4* data, const glm::ivec2& originalSize, int x, int y)
//{
//	x = glm::clamp(x, 0, originalSize.x - 1);
//	y = glm::clamp(y, 0, originalSize.y - 1);
//	return data[y * originalSize.x + x];
//}
//
//
//__global__ void GPUHermiteBicubic(uint n, glm::vec4* data, glm::vec4* buffer, glm::uvec2 originalSize, glm::uvec2 desiredSize) {
//
//	uint index = ThreadIndex1D();
//
//	if (index >= n) {
//		return;
//	}
//
//	float x = index % desiredSize.x;
//	float y = index / desiredSize.x;
//
//	float u = x / float(desiredSize.x - 1);
//	float v = y / float(desiredSize.y - 1);
//
//	float xOld = u * originalSize.x;
//	int xint = int(xOld);
//	float xfract = xOld - glm::floor(xOld);
//
//	float yOld = v * originalSize.y;
//	int yint = int(yOld);
//	float yfract = yOld - glm::floor(yOld);
//
//
//	// 1st row
//	auto p00 = GetClamped(data, originalSize, xint - 1, yint - 1);
//	auto p10 = GetClamped(data, originalSize, xint + 0, yint - 1);
//	auto p20 = GetClamped(data, originalSize, xint + 1, yint - 1);
//	auto p30 = GetClamped(data, originalSize, xint + 2, yint - 1);
//
//	// 2nd row
//	auto p01 = GetClamped(data, originalSize, xint - 1, yint + 0);
//	auto p11 = GetClamped(data, originalSize, xint + 0, yint + 0);
//	auto p21 = GetClamped(data, originalSize, xint + 1, yint + 0);
//	auto p31 = GetClamped(data, originalSize, xint + 2, yint + 0);
//
//	// 3rd row
//	auto p02 = GetClamped(data, originalSize, xint - 1, yint + 1);
//	auto p12 = GetClamped(data, originalSize, xint + 0, yint + 1);
//	auto p22 = GetClamped(data, originalSize, xint + 1, yint + 1);
//	auto p32 = GetClamped(data, originalSize, xint + 2, yint + 1);
//
//	// 4th row
//	auto p03 = GetClamped(data, originalSize, xint - 1, yint + 2);
//	auto p13 = GetClamped(data, originalSize, xint + 0, yint + 2);
//	auto p23 = GetClamped(data, originalSize, xint + 1, yint + 2);
//	auto p33 = GetClamped(data, originalSize, xint + 2, yint + 2);
//
//	glm::vec4 ret(0.0);
//	for (int i = 0; i < 3; ++i)
//	{
//		float col0 = CubicHermite(p00[i], p10[i], p20[i], p30[i], xfract);
//		float col1 = CubicHermite(p01[i], p11[i], p21[i], p31[i], xfract);
//		float col2 = CubicHermite(p02[i], p12[i], p22[i], p32[i], xfract);
//		float col3 = CubicHermite(p03[i], p13[i], p23[i], p33[i], xfract);
//		float value = CubicHermite(col0, col1, col2, col3, yfract);
//		ret[i] = glm::clamp(value, 0.0f, 1.0f);
//	}
//
//	buffer[index] = ret;
//
//}
//
//__global__ void GPUNearest(uint n, glm::vec4* data, glm::vec4* buffer, const glm::uvec2 originalSize, const glm::uvec2 desiredSize) {
//	const uint index = ThreadIndex1D();
//
//	if (index >= n) {
//		return;
//	}
//
//	const float x = index % originalSize.x;
//	const float y = index / originalSize.x;
//
//	const float u = x / float(originalSize.x);
//	const float v = y / float(originalSize.y);
//
//	const int xInt = int(u * desiredSize.x);
//	const int yInt = int(v * desiredSize.y);
//
//	const int oldIndex = yInt * desiredSize.x + xInt;
//	buffer[index] = data[oldIndex];
//
//}
//
//namespace CUDAFilter {
//
//
//	__host__ void HermiteBicubic(glm::vec4* data, glm::uvec2& originalSize, glm::uvec2& desiredSize) {
//
//		uint count = originalSize.x*originalSize.y;
//
//		uint blockSize = 64;
//		uint blockCount = (count + blockSize - 1) / blockSize;
//
//		glm::vec4* deviceBuffer;
//		CudaCheck(cudaMalloc((void**)&deviceBuffer, count * sizeof(glm::vec4)));
//
//		GPUHermiteBicubic << < blockCount, blockSize >> > (count, data, deviceBuffer, originalSize, desiredSize);
//		CudaCheck(cudaPeekAtLastError());
//		CudaCheck(cudaDeviceSynchronize());
//
//		CudaCheck(cudaMemcpy(data, deviceBuffer, count * sizeof(glm::vec4), cudaMemcpyDeviceToDevice));
//		CudaCheck(cudaFree(deviceBuffer));
//
//	}
//
//	__host__ void Nearest(glm::vec4* data, const glm::uvec2& originalSize, const glm::uvec2& desiredSize) {
//
//		uint count = originalSize.x*originalSize.y;
//
//		uint blockSize = 64;
//		uint blockCount = (count + blockSize - 1) / blockSize;
//
//		glm::vec4* deviceBuffer;
//		CudaCheck(cudaMalloc((void**)&deviceBuffer, count * sizeof(glm::vec4)));
//
//		GPUNearest << < blockCount, blockSize >> > (count, data, deviceBuffer, originalSize, desiredSize);
//		CudaCheck(cudaPeekAtLastError());
//		CudaCheck(cudaDeviceSynchronize());
//
//		CudaCheck(cudaMemcpy(data, deviceBuffer, count * sizeof(glm::vec4), cudaMemcpyDeviceToDevice));
//		CudaCheck(cudaFree(deviceBuffer));
//
//	}
//
//}