#pragma once

#include <glm/glm.hpp>
#include "Types.h"

class ComputePolicy {

public:

	//Constructors + Destructors
	ComputePolicy() = default;
	ComputePolicy(uint, uint, int, int);
	ComputePolicy(glm::uvec3, glm::uvec3, int, int);


	//Policy Helpers
	uint GetThreadCount() const;


	glm::uvec3 gridsize;
	glm::uvec3 blocksize;
	int sharedMemory;
	int stream;

};