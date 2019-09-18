#include "Tracer/Camera/Film/Film.h"
#include "stdio.h"

//results(S_BEST_DEVICE)
Film::Film(){
	resolutionRatio = 1.0f;
}

Film::~Film() {

}


//__device__ glm::vec2 Film::GetSample(uint id, curandState& randState) {
//
//	glm::vec2 parameterization = glm::vec2(
//		(curand_uniform(&randState) - 0.5f + id % resolution.x) / (resolution.x - 1),
//		(curand_uniform(&randState) - 0.5f + id / resolution.x) / (resolution.y - 1)
//	);
//
//	return parameterization;
//
//}
