#pragma once

#include "Core/Composition/Component/Component.h"

#include "Types.h"
#include <glm/glm.hpp>

class Tet : Component
{

public:

	Tet() = default;
	~Tet() = default;


	glm::uvec4 indices;
	uint material;
	uint object;

};
