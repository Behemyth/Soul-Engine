#pragma once

#include "Component.h"

#include <glm/glm.hpp>

class BoundingBox : Component
{

public:

	BoundingBox() = default;
	~BoundingBox() = default;

	glm::vec3 min;
	glm::vec3 max;

};