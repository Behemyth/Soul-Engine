#include "RenderWidget.cuh"
#include "Parallelism/Compute/CUDA/Utility/CUDAHelper.cuh"
#include "Parallelism/Compute/DeviceAPI.h"

//static uint allocatedSize = 0;
//static uint* deviceWeights = 0;
//
//__global__ void IntegrateKernal(const uint n, glm::vec4* A, glm::vec4* B, int* mask, uint* weights, const uint counter) {
//
//
//	uint index = ThreadIndex1D();
//
//	if (index >= n) {
//		return;
//	}
//
//	if (mask[index] > 0) {
//		weights[index]++;
//
//		B[index] = glm::mix(B[index], A[index], 1.0f / weights[index]);
//	}
//
//	A[index] = B[index];
//
//}
//
//
//__host__ void Integrate(uint size, glm::vec4* A, glm::vec4* B, int* mask, const uint counter) {
//
//	if (size > allocatedSize) {
//		allocatedSize = size;
//		CudaCheck(cudaMalloc((void**)&deviceWeights, size * sizeof(uint)));
//	}
//
//	//clear b and deviceWeights
//	if (counter == 1) {
//		cudaMemset(deviceWeights, 0, size * sizeof(uint));
//		cudaMemset(B, 0, size * sizeof(glm::vec4));
//	}
//
//	uint blockSize = 64;
//	uint gridSize = (size + blockSize - 1) / blockSize;
//
//	IntegrateKernal << <gridSize, blockSize >> > (size, A, B, mask, deviceWeights, counter);
//	CudaCheck(cudaPeekAtLastError());
//	CudaCheck(cudaDeviceSynchronize());
//
//}