#pragma once

#include "Types.h"
#include "Core/Composition/Component/Component.h"

#include <glm/glm.hpp>


class Vertex : Component
{

public:

	Vertex() = default;
	~Vertex() = default;


	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textureCoord;
	glm::vec3 velocity;

	uint object;

};

class GUIVertex : Component {

public:

	GUIVertex() = default;
	~GUIVertex() = default;


	glm::vec2 position;
	glm::vec2 textureCoord;
	uint colour;

};