#pragma once

#include "Component.h"
#include "Types.h"

#include <glm/glm.hpp>

class Face : Component
{

public:

	Face() = default;
	~Face() = default;

	glm::uvec3 indices;
	uint material; //TODO investigate materials
};


//Moller-Trumbore
//__host__ __device__ __inline__ bool FindTriangleIntersect(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
//	const glm::vec3& rayO, const glm::vec3& rayD, const glm::vec3& invDir,
//	float& t, const float& tMax, float& bary1, float& bary2)
//{
//
//	glm::vec3 edge1 = b - a;
//	glm::vec3 edge2 = c - a;
//
//	glm::vec3 pvec = glm::cross(rayD, edge2);
//
//	float det = glm::dot(edge1, pvec);
//
//	if (det == 0.f) {
//		return false;
//	}
//
//	float inv_det = 1.0f / det;
//
//	glm::vec3 tvec = rayO - a;
//
//	bary1 = glm::dot(tvec, pvec) * inv_det;
//
//	glm::vec3 qvec = glm::cross(tvec, edge1);
//
//	bary2 = glm::dot(rayD, qvec) * inv_det;
//
//	t = glm::dot(edge2, qvec) * inv_det;
//
//	return(t > EPSILON &&t < tMax && (bary1 >= 0.0f && bary2 >= 0.0f && (bary1 + bary2) <= 1.0f));
//}