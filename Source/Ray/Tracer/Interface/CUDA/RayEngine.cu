#include "Tracer/CUDA/RayEngine.cuh"

//#include <cuda_runtime.h>
//#include <curand_kernel.h>
//#include "Parallelism/Compute/CUDA/Utility/CUDAHelper.cuh"
//#include "Parallelism/Compute/DeviceAPI.h"
//#include "Core/Scene/Bounding Volume Heirarchy/CUDA/BVH.cuh"
//
//#define RAY_BIAS_DISTANCE 0.0002f 
//
//namespace RayEngineCUDA {
//
//
//	__global__ void RandomSetup(uint n, curandState* randomState, uint raySeed) {
//
//
//		uint index = ThreadIndex1D();
//
//		if (index >= n) {
//			return;
//		}
//
//		curandState randState;
//		curand_init(index, 0, 0, &randState);
//
//		randomState[index] = randState;
//
//	}
//
//	__global__ void EngineSetup(uint n, RayJob* jobs, int jobSize) {
//
//
//		const int index = ThreadIndex1D();
//
//		if (index >= n) {
//			return;
//		}
//
//		const int startIndex = 0;
//
//		int cur = 0;
//		((glm::vec4*)jobs[cur].camera.film.results)[index - startIndex] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
//		jobs[cur].camera.film.hits[index - startIndex] = 0;
//
//	}
//
//	__global__ void RaySetup(uint n, uint jobSize, RayJob* job, Ray* rays, int* nAtomic, curandState* randomState) {
//
//		const uint index = ThreadIndex1D();
//
//		if (index >= n) {
//			return;
//		}
//
//		const auto startIndex = 0;
//		const auto cur = 0;
//
//		auto samples = job[cur].samples;
//		const uint sampleIndex = (index - startIndex) / glm::ceil(samples); //the index of the pixel / sample
//		const auto localIndex = (index - startIndex) % static_cast<int>(glm::ceil(samples));
//
//		curandState randState = randomState[index];
//
//		if (localIndex + 1 <= samples || curand_uniform(&randState) < __fsub_rd(samples, glm::floor(samples))) {
//
//			Ray ray;
//			ray.storage = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
//			ray.resultOffset = sampleIndex;
//			glm::vec3 orig;
//			glm::vec3 dir;
//			//job[cur].camera.GenerateRay(sampleIndex, orig, dir, randState);
//			ray.origin = glm::vec4(orig, 0.0f);
//			ray.direction = glm::vec4(dir, 4000000000000.0f);
//
//			atomicAdd(job[cur].camera.film.hits + sampleIndex, 1);
//
//			const auto val = FastAtomicAdd(nAtomic);
//			rays[val] = ray;
//
//		}
//
//		randomState[index] = randState;
//
//	}
//
//	__host__ __device__ __inline__ glm::vec3 PositionAlongRay(const Ray& ray, const float& t) {
//		return glm::vec3(ray.origin.x, ray.origin.y, ray.origin.z) + t * glm::vec3(ray.direction.x, ray.direction.y, ray.direction.z);
//	}
//
//
//	__global__ void ProcessHits(uint n, RayJob* job, int jobSize, Ray* rays, Ray* raysNew, Sky* sky, Face* faces, Vertex* vertices, Material* materials, int * nAtomic, curandState* randomState) {
//
//		uint index = ThreadIndex1D();
//
//		if (index >= n) {
//			return;
//		}
//
//		int cur = 0;
//
//		Ray ray = rays[index];
//
//		uint faceHit = ray.currentHit;
//
//		float hitT = ray.direction.w;
//
//		uint localIndex = ray.resultOffset;
//
//		curandState randState = randomState[index];
//
//		glm::vec3 col;
//
//		if (faceHit == uint(-1)) {
//
//			//col = glm::vec3(ray.storage.x, ray.storage.y, ray.storage.z)*sky->ExtractColour({ ray.direction.x, ray.direction.y, ray.direction.z });
//
//		}
//		else {
//
//			Face face = faces[faceHit];
//
//			Material mat = materials[face.material];
//
//			glm::vec2 bary = ray.bary;
//
//			glm::vec3 n0 = vertices[face.indices.x].position;
//			glm::vec3 n1 = vertices[face.indices.y].position;
//			glm::vec3 n2 = vertices[face.indices.z].position;
//
//			glm::vec3 bestNormal = glm::normalize(glm::cross(n1 - n0, n2 - n0));
//
//			/*glm::vec3 n0 = vertices[faceHit->indices.x].normal;
//			glm::vec3 n1 = vertices[faceHit->indices.y].normal;
//			glm::vec3 n2 = vertices[faceHit->indices.z].normal;
//
//			glm::vec3 bestNormal = (1 - bary.x - bary.y) * n0 + bary.x * n1 + bary.y * n2;*/
//
//			glm::vec2 uv0 = vertices[face.indices.x].textureCoord;
//			glm::vec2 uv1 = vertices[face.indices.y].textureCoord;
//			glm::vec2 uv2 = vertices[face.indices.z].textureCoord;
//
//			glm::vec2 bestUV = (1.0f - bary.x - bary.y) * uv0 + bary.x * uv1 + bary.y * uv2;
//
//			glm::vec3 orientedNormal = glm::dot(bestNormal, glm::vec3(ray.direction.x, ray.direction.y, ray.direction.z)) < 0 ? bestNormal : bestNormal * -1.0f;
//
//			glm::vec3 biasVector = (RAY_BIAS_DISTANCE * orientedNormal);
//
//			glm::vec3 bestIntersectionPoint = PositionAlongRay(ray, hitT);
//
//			glm::vec3 accumulation;
//			accumulation.x = ray.storage.x * mat.emit.x;
//			accumulation.y = ray.storage.y * mat.emit.y;
//			accumulation.z = ray.storage.z * mat.emit.z;
//
//			//tex2D<float>(texObj, tu, tv)
//			//unsigned char blue = tex2D<unsigned char>(mat->texObj, (4 * localIndex), localIndex);
//			//unsigned char green = tex2D<unsigned char>(mat->texObj, (4 * localIndex) + 1, localIndex);
//			//unsigned char red = tex2D<unsigned char>(mat->texObj, (4 * localIndex) + 2, localIndex);
//
//			//float4 PicCol = tex2DLod<float4>(mat.diffuseImage.texObj, bestUV.x, bestUV.y, 0.0f);
//			//float PicCol = tex2D<float>(mat->texObj, bestUV.x * 50, bestUV.y * 50);
//			//ray.storage *= glm::vec4(PicCol.x, PicCol.y, PicCol.z, 1.0f);
//
//			//ray.storage *= mat->diffuse;
//
//			float r1 = 2 * PI * curand_uniform(&randState);
//			float r2 = curand_uniform(&randState);
//			float r2s = sqrtf(r2);
//
//			glm::vec3 u = glm::normalize(glm::cross((glm::abs(orientedNormal.x) > .1 ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0)), orientedNormal));
//			glm::vec3 v = glm::cross(orientedNormal, u);
//
//			ray.origin = glm::vec4(bestIntersectionPoint + biasVector, 0.0f);
//			ray.direction = glm::vec4(glm::normalize(u*cos(r1)*r2s + v * sin(r1)*r2s + orientedNormal * sqrtf(1 - r2)), 40000000000000.0f);
//
//			col = accumulation;
//
//			raysNew[FastAtomicAdd(nAtomic)] = ray;
//
//			//update the state
//			randomState[index] = randState;
//		}
//
//		int samples = job[cur].camera.film.hits[localIndex];
//
//		col /= samples;
//
//		glm::vec4* pt = &((glm::vec4*)job[cur].camera.film.results)[localIndex];
//
//		atomicAdd(&(pt->x), col.x);
//
//		atomicAdd(&(pt->y), col.y);
//
//		atomicAdd(&(pt->z), col.z);
//
//
//	}
//
//	__global__ void ExecuteJobs(uint n, Ray* rays, BVH* bvhP, Vertex* vertices, Face* faces, BoundingBox* boxes, int* counter) {
//
//		BVH bvh = *bvhP;
//
//		Ray ray;
//		int rayidx;
//
//		extern __shared__ volatile int nextRayArray[]; // Current ray index in global buffer needs the (max) block height. 
//
//		do {
//			const uint tidx = threadIdx.x;
//			volatile int& rayBase = nextRayArray[threadIdx.y];
//
//
//			const bool          terminated = bvh.IsTerminated();
//			const unsigned int  maskTerminated = __ballot(terminated);
//			const int           numTerminated = __popc(maskTerminated);
//			const int           idxTerminated = __popc(maskTerminated & ((1u << tidx) - 1));
//
//			//fetch a new ray
//
//			if (terminated)
//			{
//				if (idxTerminated == 0) {
//					rayBase = atomicAdd(counter, numTerminated);
//				}
//
//				rayidx = rayBase + idxTerminated;
//				if (rayidx >= n) {
//					break;
//				}
//
//				//ray local storage + precalculations
//				ray = rays[rayidx];
//
//				ray.currentHit = static_cast<uint>(-1);
//				bvh.ResetTraversal(ray);
//			}
//
//			bvh.Traverse(ray,vertices,faces);
//
//			//update the data
//			rays[rayidx] = ray;
//
//		} while (true);
//	}
//}