#pragma once

//#include "Core/Material/Texture/Image.cuh"

#include <glm/glm.hpp>
#include <string>

class Material {
public:

	Material(std::string texName = "Resources//Textures//SoulDefault.png");

	glm::vec4 diffuse;
	glm::vec4 emit;

	//Image diffuseImage;

};
