#pragma once

//#include "Core/Material/Texture/Image.cuh"
#include <string>
#include <glm/glm.hpp>
	
class Sky
{

public:
	Sky();
	Sky(std::string); 

	void UpdateSky();

	//__device__ glm::vec3 ExtractColour(const glm::vec3& direction);

	//Image* image;

private:


	
};


