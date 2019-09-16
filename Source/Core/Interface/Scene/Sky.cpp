#include "Sky.h"

Sky::Sky(std::string texName) {

	//image = new Image();

	//CudaCheck(cudaDeviceSynchronize());
	//image->LoadFromFile(texName.c_str(),true,false);
	//CudaCheck(cudaDeviceSynchronize());
}

//__device__ glm::vec3 Sky::ExtractColour(const glm::vec3& direction){
//
//	/*float theta = 0.5f + atan2f(direction.z, direction.x)/(2 * PI);
//	float gamma = 0.5f - asinf(direction.y)/ PI;
//	float4 col = tex2D<float4>(image->texObj, theta, gamma );*/
//
//
//	float normalized = direction.y / 2.0f + 0.5f;
//	return glm::mix(glm::vec3(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f),glm::vec3(135 / 255.0f, 200 / 255.0f, 240 / 255.0f),normalized/2.0f+0.5f)*0.5f;
//}
