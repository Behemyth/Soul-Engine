#include "ComputePolicy.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/component_wise.hpp"

ComputePolicy::ComputePolicy(uint size_, uint blockSize_, int sharedBytes, int stream_) :
	gridsize((size_ + blockSize_ - 1) / blockSize_, 1, 1), 
	blocksize(blockSize_, 1, 1),
	sharedMemory(sharedBytes), 
	stream(stream_)
{

}

ComputePolicy::ComputePolicy(glm::uvec3 grid, glm::uvec3 block, int sharedBytes, int stream_) :
	gridsize(grid), 
	blocksize(block), 
	sharedMemory(sharedBytes), 
	stream(stream_) 
{

}

uint ComputePolicy::GetThreadCount() const{
	return glm::compMul(gridsize)*glm::compMul(blocksize);
}
