#pragma once

#include "Types.h"
#include <glm/glm.hpp>

class Ray
{

public:

	glm::vec4 storage;
	glm::vec4 origin;
	glm::vec4 direction;
	glm::vec2 bary;
	uint currentHit;
	uint resultOffset;
	char job;

};