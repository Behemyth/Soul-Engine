#pragma once

//#include "Core/Scene/Bounding Volume Heirarchy/Node.h"
//#include "Tracer/Ray.h"
//#include "Core/Geometry/Face.h"
//#include "Core/Geometry/Vertex.h"
//#include "Types.h"
//
//#include <cuda.h>
//#include <cuda_runtime.h>
//
//#define STACK_SIZE 64
//#define DYNAMIC_FETCH_THRESHOLD 20          // If fewer than this active, fetch new rays
//
////cant have AABB defined and not define WOOP_TRI
////#define WOOP_AABB
//
//class BVH {
//
//public:
//
//	BVH() = default;
//	__device__ BVH(const BVH&);
//
//	BVH& operator=(const BVH& other) = delete;
//
//	__device__ bool IsLeaf(std::uint32_t) const;
//	__device__ bool IsTerminated() const;
//	__device__ void ResetTraversal(const Ray&);
//	__device__ void Traverse(Ray&, Vertex*, Face*);
//
//	std::uint32_t root;
//	std::uint32_t leafSize;
//	Node* nodes;
//	BoundingBox* boxes;
//
//
//private:
//
//	//precalculated information
//	std::uint32_t traversalStack[STACK_SIZE];
//
//	char    stackPtr;                       // Current position in traversal stack.
//	std::uint32_t   currentLeaf;                       // First postponed leaf, non-negative if none.
//	std::uint32_t   currentNode;				// Non-negative: current internal node, negative: second postponed leaf.
//
//
//#if defined WOOP_AABB
//
//	float idirxNear;
//	float idiryNear;
//	float idirzNear;
//	float idirxFar;
//	float idiryFar;
//	float idirzFar;
//
//#else
//
//	float oodx;
//	float oody;
//	float oodz;
//
//	float idirx;
//	float idiry;
//	float idirz;
//
//#endif
//
//	//ray precalc
//	std::int32_t kz;
//	std::int32_t kx;
//	std::int32_t ky;
//	float Sx;
//	float Sy;
//	float Sz;
//
//};
//
//__global__ void BuildTree(std::uint32_t, BVH*, Node*, std::int64_t*, BoundingBox*);
//__global__ void Reset(std::uint32_t, Node*);
