module synodic.soul.tracer;

import std;

//results(S_BEST_DEVICE)
Film::Film(){
	resolutionRatio = 1.0f;
}

Film::~Film() {

}


//__device__ glm::vec2 Film::GetSample(std::uint32_t id, curandState& randState) {
//
//	glm::vec2 parameterization = glm::vec2(
//		(curand_uniform(&randState) - 0.5f + id % resolution.x) / (resolution.x - 1),
//		(curand_uniform(&randState) - 0.5f + id / resolution.x) / (resolution.y - 1)
//	);
//
//	return parameterization;
//
//}
