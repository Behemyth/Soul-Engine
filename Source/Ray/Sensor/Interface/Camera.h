#pragma once

#include "Tracer/Camera/Film/Film.h"
//#include <curand_kernel.h>

#include "Types.h"
#include "glm/glm.hpp"

class Camera {
public:
	Camera();
	~Camera();

	//Given a positive integer, this function fills in the given ray's values based on the camera's position orientation and lens.
	//__device__ void SetupRay(const uint&, Ray&, thrust::default_random_engine&, thrust::uniform_real_distribution<float>&);
	//__device__ void GenerateRay(const uint, glm::vec3&, glm::vec3&, curandState&);

	void OffsetOrientation(float x, float y);

	void UpdateVariables();

	float aspectRatio;
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec2 fieldOfView;
	Film film;
	
private:
	float aperture;
	float focalDistance;

	//VARIABLE PRECALC
	glm::vec3 verticalAxis;
	glm::vec3 yHelper;
	glm::vec3 xHelper;


};
