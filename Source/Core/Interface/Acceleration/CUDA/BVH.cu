#include "BVH.cuh"

//#include "Parallelism/Compute/DeviceAPI.h"
//#include <glm/glm.hpp>
//
//#define eps 0.00000000000000000000000082718061
//#define terminator static_cast<uint>(-1)
//
//#define p (1.0f + 2e-23f)
//#define m (1.0f - 2e-23f)
//
//__inline__ __device__ float up(float a) { return a > 0.0f ? a * p : a * m; }
//__inline__ __device__ float dn(float a) { return a > 0.0f ? a * m : a * p; }
//
//__inline__ __device__ float Up(float a) { return a * p; }
//__inline__ __device__ glm::vec3 Up(glm::vec3 a) { return a * p; }
//__inline__ __device__ float Dn(float a) { return a * m; }
//__inline__ __device__ glm::vec3 Dn(glm::vec3 a) { return a * m; }
//
//__device__ __inline__ int __float_as_int(float in) {
//	union fi { int i; float f; } conv;
//	conv.f = in;
//	return conv.i;
//}
//
//__device__ __inline__ float __int_as_float(int a)
//
//{
//
//	union { int a; float b; } u;
//
//	u.a = a;
//
//	return u.b;
//
//}
//
//__device__ __inline__ float min4(float a, float b, float c, float d)
//{
//	return fminf(fminf(fminf(a, b), c), d);
//}
//
//__device__ __inline__ float max4(float a, float b, float c, float d)
//{
//	return fmaxf(fmaxf(fmaxf(a, b), c), d);
//}
//
//__device__ __inline__ float min3(float a, float b, float c)
//{
//	return fminf(fminf(a, b), c);
//}
//
//__device__ __inline__ float max3(float a, float b, float c)
//{
//	return fmaxf(fmaxf(a, b), c);
//}
//
//// Using integer min,max
//__inline__ __device__ float fminf2(float a, float b)
//{
//	int a2 = __float_as_int(a);
//	int b2 = __float_as_int(b);
//	return __int_as_float(a2 < b2 ? a2 : b2);
//}
//
//__inline__ __device__ float fmaxf2(float a, float b)
//{
//	int a2 = __float_as_int(a);
//	int b2 = __float_as_int(b);
//	return __int_as_float(a2 > b2 ? a2 : b2);
//}
//
//// Using video instructions
//__device__ __inline__ int   min_min(int a, int b, int c) {
//	int v;
//	asm("vmin.s32.s32.s32.min %0, %1, %2, %3;"
//		: "=r"(v) : "r"(a), "r"(b), "r"(c));
//	return v;
//}
//__device__ __inline__ int   min_max(int a, int b, int c) { int v; asm("vmin.s32.s32.s32.max %0, %1, %2, %3;" : "=r"(v) : "r"(a), "r"(b), "r"(c)); return v; }
//__device__ __inline__ int   max_min(int a, int b, int c) { int v; asm("vmax.s32.s32.s32.min %0, %1, %2, %3;" : "=r"(v) : "r"(a), "r"(b), "r"(c)); return v; }
//__device__ __inline__ int   max_max(int a, int b, int c) { int v; asm("vmax.s32.s32.s32.max %0, %1, %2, %3;" : "=r"(v) : "r"(a), "r"(b), "r"(c)); return v; }
//__device__ __inline__ float fmin_fmin(float a, float b, float c) { return __int_as_float(min_min(__float_as_int(a), __float_as_int(b), __float_as_int(c))); }
//__device__ __inline__ float fmin_fmax(float a, float b, float c) { return __int_as_float(min_max(__float_as_int(a), __float_as_int(b), __float_as_int(c))); }
//__device__ __inline__ float fmax_fmin(float a, float b, float c) { return __int_as_float(max_min(__float_as_int(a), __float_as_int(b), __float_as_int(c))); }
//__device__ __inline__ float fmax_fmax(float a, float b, float c) { return __int_as_float(max_max(__float_as_int(a), __float_as_int(b), __float_as_int(c))); }
//
//
//__device__ __inline__ float magic_max7(float a0, float a1, float b0, float b1, float c0, float c1, float d)
//{
//	const float t1 = fmin_fmax(a0, a1, d);
//	const float t2 = fmin_fmax(b0, b1, t1);
//	const float t3 = fmin_fmax(c0, c1, t2);
//	return t3;
//}
//
//__device__ __inline__ float magic_min7(float a0, float a1, float b0, float b1, float c0, float c1, float d)
//{
//	const float t1 = fmax_fmin(a0, a1, d);
//	const float t2 = fmax_fmin(b0, b1, t1);
//	const float t3 = fmax_fmin(c0, c1, t2);
//	return t3;
//}
//
//__device__ __inline__ float spanBeginKepler(float a0, float a1, float b0, float b1, float c0, float c1, float d) { return fmax_fmax(fminf(a0, a1), fminf(b0, b1), fmin_fmax(c0, c1, d)); }
//__device__ __inline__ float spanEndKepler(float a0, float a1, float b0, float b1, float c0, float c1, float d) { return fmin_fmin(fmaxf(a0, a1), fmaxf(b0, b1), fmax_fmin(c0, c1, d)); }
//
////woop
//__host__ __device__ __inline__ bool FindTriangleIntersect(const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC,
//	const glm::vec3& rayO, int kx, int ky, int kz, float Sx, float Sy, float Sz,
//	float& t, const float& tMax, float& bary1, float& bary2)
//{
//	const glm::vec3 A = triA - rayO;
//	const glm::vec3 B = triB - rayO;
//	const glm::vec3 C = triC - rayO;
//
//	const float Ax = A[kx] - Sx * A[kz];
//	const float Ay = A[ky] - Sy * A[kz];
//	const float Bx = B[kx] - Sx * B[kz];
//	const float By = B[ky] - Sy * B[kz];
//	const float Cx = C[kx] - Sx * C[kz];
//	const float Cy = C[ky] - Sy * C[kz];
//
//	float U = Cx * By - Cy * Bx;
//	float V = Ax * Cy - Ay * Cx;
//	float W = Bx * Ay - By * Ax;
//
//	if (U == 0.0f || V == 0.0f || W == 0.0f) {
//		const double CxBy = static_cast<double>(Cx)*static_cast<double>(By);
//		const double CyBx = static_cast<double>(Cy)*static_cast<double>(Bx);
//		U = static_cast<float>(CxBy - CyBx);
//		const double AxCy = static_cast<double>(Ax)*static_cast<double>(Cy);
//		const double AyCx = static_cast<double>(Ay)*static_cast<double>(Cx);
//		V = static_cast<float>(AxCy - AyCx);
//		const double BxAy = static_cast<double>(Bx)*static_cast<double>(Ay);
//		const double ByAx = static_cast<double>(By)*static_cast<double>(Ax);
//		W = static_cast<float>(BxAy - ByAx);
//	}
//
//	if ((U < 0.0f || V < 0.0f || W < 0.0f) &&
//		(U > 0.0f || V > 0.0f || W > 0.0f)) {
//		return false;
//	}
//
//	const float det = U + V + W;
//
//	if (det == 0.0f) {
//		return false;
//	}
//
//	const float Az = Sz * A[kz];
//	const float Bz = Sz * B[kz];
//	const float Cz = Sz * C[kz];
//	const float T = U * Az + V * Bz + W * Cz;
//
//	/*int det_sign = glm::sign(det);
//	if (xorf(T, det_sign) < 0.0f) ||
//	xorf(T, det_sign) > hit.t * xorf(det, det_sign)){
//	return false;
//	}*/
//
//	const float rcpDet = 1.0f / det;
//
//	bary1 = V * rcpDet;
//	bary2 = W * rcpDet;
//	t = T * rcpDet;
//
//	return (t > EPSILON &&t < tMax);
//}
//
//// Returns the highest differing bit of i and i+1
//__device__ uint HighestBit(uint i, uint64* morton)
//{
//	return morton[i] ^ morton[i + 1];
//}
//
//__global__ void BuildTree(uint n, BVH* data, Node* nodes, uint64* mortonCodes, BoundingBox* boxes)
//{
//	const uint index = ThreadIndex1D();
//
//	if (index >= n) {
//		return;
//	}
//
//
//	uint nodeID;
//	const uint innerSize = n - 1;
//
//	//first process the leaf parents
//	if (index == 0 || index < innerSize &&
//		HighestBit(index - 1, mortonCodes) > HighestBit(index, mortonCodes))
//	{
//
//		// parent = right, set parent left child and range to node	
//		const uint parentID = index;
//		Node& parentNode = nodes[parentID];
//
//		parentNode.childLeft = index + n;
//		parentNode.rangeLeft = index;
//		nodeID = parentID;
//
//	}
//	else
//	{
//
//		// parent = left -1, set parent right child and range to node
//		const uint parentID = index - 1;
//		Node& parentNode = nodes[parentID];
//
//		parentNode.childRight = index + n;
//		parentNode.rangeRight = index;
//		nodeID = parentID;
//
//	}
//
//	//next iterate until the root is hit
//	while (true) {
//
//		//only process first thread at the node
//		if (atomicAdd(&nodes[nodeID].atomic, 1) != 1) {
//			return;
//		}
//
//		//only one thread is here, read in the node
//		Node node = nodes[nodeID];
//
//		//TODO store bounding boxes to cut 2 global reads
//		//combine the bounding boxes
//		const BoundingBox boxLeft = node.childLeft >= n ? boxes[node.childLeft - n] : nodes[node.childLeft].box;
//		const BoundingBox boxRight = node.childRight >= n ? boxes[node.childRight - n] : nodes[node.childRight].box;
//
//		node.box.max = glm::max(boxLeft.max, boxRight.max);
//		node.box.min = glm::min(boxLeft.min, boxRight.min);
//
//		nodes[nodeID].box = node.box;
//
//		if (node.rangeLeft == 0 && node.rangeRight == innerSize) {
//			data->root = nodeID;
//			return;
//		}
//
//		if (node.rangeLeft == 0 || node.rangeRight < innerSize &&
//			HighestBit(node.rangeLeft - 1, mortonCodes) > HighestBit(node.rangeRight, mortonCodes))
//		{
//
//			// parent = right, set parent left child and range to node		
//			const uint parentID = node.rangeRight;
//			Node& parentNode = nodes[parentID];
//
//			parentNode.childLeft = nodeID;
//			parentNode.rangeLeft = node.rangeLeft;
//			nodeID = parentID;
//
//		}
//		else
//		{
//
//			// parent = left -1, set parent right child and range to node
//			const uint parentID = node.rangeLeft - 1;
//			Node& parentNode = nodes[parentID];
//
//			parentNode.childRight = nodeID;
//			parentNode.rangeRight = node.rangeRight;
//			nodeID = parentID;
//
//		}
//	}
//}
//
//__global__ void Reset(uint n, Node* nodes)
//{
//
//	const uint index = ThreadIndex1D();
//
//	if (index >= n) {
//		return;
//	}
//
//	//inner node
//	Node tempInner;
//	tempInner.atomic = 0; //inner nodes are not visited
//	nodes[index] = tempInner;
//
//}
//
//__device__ BVH::BVH(const BVH& other) {
//
//	currentNode = terminator;
//	traversalStack[0] = terminator;
//
//	root = other.root;
//	leafSize = other.leafSize;
//	nodes = other.nodes;
//	boxes = other.boxes;
//}
//
//__device__ bool BVH::IsLeaf(uint test) const {
//	return test >= leafSize && test < leafSize * 2;
//}
//__device__ bool BVH::IsTerminated() const {
//	return currentNode == terminator;
//}
//
//__device__ void BVH::ResetTraversal(const Ray& ray) {
//
//	//triangle precalc
//	const glm::vec3 absDir = glm::abs(ray.direction);
//
//	if (absDir.x >= absDir.y&&absDir.x >= absDir.z) {
//		kz = 0;
//	}
//	else if (absDir.y >= absDir.x&&absDir.y >= absDir.z) {
//		kz = 1;
//	}
//	else {
//		kz = 2;
//	}
//
//	kx = kz + 1;
//
//	if (kx == 3) {
//		kx = 0;
//	}
//
//	ky = kx + 1;
//
//	if (ky == 3) {
//		ky = 0;
//	}
//
//	if (ray.direction[kz] < 0.0f) {
//		Swap(kx, ky);
//	}
//
//	Sx = ray.direction[kx] / ray.direction[kz];
//	Sy = ray.direction[ky] / ray.direction[kz];
//	Sz = 1.0f / ray.direction[kz];
//
//#if defined WOOP_AABB
//
//	glm::vec3 rdir = 1.0f / ray.direction;
//	idirxNear = Dn(Dn(rdir[kx]));
//	idiryNear = Dn(Dn(rdir[ky]));
//	idirzNear = Dn(Dn(rdir[kz]));
//	idirxFar = Up(Up(rdir[kx]));
//	idiryFar = Up(Up(rdir[ky]));
//	idirzFar = Up(Up(rdir[kz]));
//
//#else
//
//	idirx = 1.0f / (fabsf(ray.direction.x) > eps ? ray.direction.x : copysignf(eps, ray.direction.x));
//	idiry = 1.0f / (fabsf(ray.direction.y) > eps ? ray.direction.y : copysignf(eps, ray.direction.y));
//	idirz = 1.0f / (fabsf(ray.direction.z) > eps ? ray.direction.z : copysignf(eps, ray.direction.z));
//
//	//non-moded
//	oodx = ray.origin.x * idirx;
//	oody = ray.origin.y * idiry;
//	oodz = ray.origin.z * idirz;
//
//#endif
//
//	stackPtr = 0;
//	currentLeaf = terminator;   // No postponed leaf.
//	currentNode = root;   // Start from the root.
//}
//
//__device__ void BVH::Traverse(Ray& ray, Vertex* vertices, Face* faces) {
//
//	//While the current node is either a leaf or an inner noed
//	while (currentNode != terminator)
//	{
//		// Until all threads find a leaf, traverse
//		while (!IsLeaf(currentNode) && currentNode != terminator)
//		{
//			// Fetch AABBs of the two child nodes
//			uint childL = nodes[currentNode].childLeft;
//			uint childR = nodes[currentNode].childRight;
//
//			const BoundingBox boxLeft = IsLeaf(childL) ? boxes[childL - leafSize] : nodes[childL].box;
//			const BoundingBox boxRight = IsLeaf(childR) ? boxes[childR - leafSize] : nodes[childR].box;
//
//
//#if defined WOOP_AABB
//
//			//grab the modifyable bounds
//			float nearX0 = boxLeft.min[kx], farX0 = boxLeft.max[kx];
//			float nearY0 = boxLeft.min[ky], farY0 = boxLeft.max[ky];
//			float nearZ0 = boxLeft.min[kz], farZ0 = boxLeft.max[kz];
//			float nearX1 = boxRight.min[kx], farX1 = boxRight.max[kx];
//			float nearY1 = boxRight.min[ky], farY1 = boxRight.max[ky];
//			float nearZ1 = boxRight.min[kz], farZ1 = boxRight.max[kz];
//
//			if (ray.direction[kx] < 0.0f) swap(nearX0, farX0);
//			if (ray.direction[ky] < 0.0f) swap(nearY0, farY0);
//			if (ray.direction[kz] < 0.0f) swap(nearZ0, farZ0);
//			if (ray.direction[kx] < 0.0f) swap(nearX1, farX1);
//			if (ray.direction[ky] < 0.0f) swap(nearY1, farY1);
//			if (ray.direction[kz] < 0.0f) swap(nearZ1, farZ1);
//
//			glm::vec3 lower0 = Dn(glm::abs(glm::vec3(ray.origin) - boxLeft.min));
//			glm::vec3 upper0 = Up(glm::abs(glm::vec3(ray.origin) - boxLeft.max));
//			glm::vec3 lower1 = Dn(abs(glm::vec3(ray.origin) - boxRight.min));
//			glm::vec3 upper1 = Up(abs(glm::vec3(ray.origin) - boxRight.max));
//
//			float max_z0 = glm::max(lower0[kz], upper0[kz]);
//			float max_z1 = glm::max(lower1[kz], upper1[kz]);
//
//			//calc the rror and update the origin
//			float err_near_x0 = Up(lower0[kx] + max_z0);
//			float err_near_y0 = Up(lower0[ky] + max_z0);
//			float err_near_x1 = Up(lower1[kx] + max_z1);
//			float err_near_y1 = Up(lower1[ky] + max_z1);
//
//			float oodxNear0 = up(ray.origin[kx] + Up(eps*err_near_x0));
//			float oodyNear0 = up(ray.origin[ky] + Up(eps*err_near_y0));
//			float oodzNear0 = ray.origin[kz];
//			float oodxNear1 = up(ray.origin[kx] + Up(eps*err_near_x1));
//			float oodyNear1 = up(ray.origin[ky] + Up(eps*err_near_y1));
//			float oodzNear1 = ray.origin[kz];
//
//			float err_far_x0 = Up(upper0[kx] + max_z0);
//			float err_far_y0 = Up(upper0[ky] + max_z0);
//			float err_far_x1 = Up(upper1[kx] + max_z1);
//			float err_far_y1 = Up(upper1[ky] + max_z1);
//
//			float oodxFar0 = dn(ray.origin[kx] - Up(eps*err_far_x0));
//			float oodyFar0 = dn(ray.origin[ky] - Up(eps*err_far_y0));
//			float oodzFar0 = ray.origin[kz];
//			float oodxFar1 = dn(ray.origin[kx] - Up(eps*err_far_x1));
//			float oodyFar1 = dn(ray.origin[ky] - Up(eps*err_far_y1));
//			float oodzFar1 = ray.origin[kz];
//
//			if (ray.direction[kx] < 0.0f) swap(oodxNear0, oodxFar0);
//			if (ray.direction[ky] < 0.0f) swap(oodyNear0, oodyFar0);
//			if (ray.direction[kx] < 0.0f) swap(oodxNear1, oodxFar1);
//			if (ray.direction[ky] < 0.0f) swap(oodyNear1, oodyFar1);
//
//
//
//			// Intersect the ray against the child nodes.
//			const float c0lox = (nearX0 - oodxNear0) * idirxNear;
//			const float c0hix = (farX0 - oodxFar0) * idirxFar;
//			const float c0loy = (nearY0 - oodyNear0) * idiryNear;
//			const float c0hiy = (farY0 - oodyFar0) * idiryFar;
//			const float c0loz = (nearZ0 - oodzNear0)   * idirzNear;
//			const float c0hiz = (farZ0 - oodzFar0)   * idirzFar;
//			const float c1loz = (nearZ1 - oodzNear1)   * idirzNear;
//			const float c1hiz = (farZ1 - oodzFar1)   * idirzFar;
//			const float c0min = spanBeginKepler(c0lox, c0hix, c0loy, c0hiy, c0loz, c0hiz, ray.origin.w);
//			const float c0max = spanEndKepler(c0lox, c0hix, c0loy, c0hiy, c0loz, c0hiz, ray.direction.w);
//
//			const float c1lox = (nearX1 - oodxNear1) * idirxNear;
//			const float c1hix = (farX1 - oodxFar1) * idirxFar;
//			const float c1loy = (nearY1 - oodyNear1) * idiryNear;
//			const float c1hiy = (farY1 - oodyFar1) * idiryFar;
//			const float c1min = spanBeginKepler(c1lox, c1hix, c1loy, c1hiy, c1loz, c1hiz, ray.origin.w);
//			const float c1max = spanEndKepler(c1lox, c1hix, c1loy, c1hiy, c1loz, c1hiz, ray.direction.w);
//
//#else
//
//			const float c0lox = boxLeft.min.x * idirx - oodx;
//			const float c0hix = boxLeft.max.x * idirx - oodx;
//			const float c0loy = boxLeft.min.y * idiry - oody;
//			const float c0hiy = boxLeft.max.y * idiry - oody;
//			const float c0loz = boxLeft.min.z   * idirz - oodz;
//			const float c0hiz = boxLeft.max.z   * idirz - oodz;
//			const float c1loz = boxRight.min.z   * idirz - oodz;
//			const float c1hiz = boxRight.max.z   * idirz - oodz;
//			const float c0min = spanBeginKepler(c0lox, c0hix, c0loy, c0hiy, c0loz, c0hiz, ray.origin.w);
//			const float c0max = spanEndKepler(c0lox, c0hix, c0loy, c0hiy, c0loz, c0hiz, ray.direction.w);
//			const float c1lox = boxRight.min.x * idirx - oodx;
//			const float c1hix = boxRight.max.x * idirx - oodx;
//			const float c1loy = boxRight.min.y * idiry - oody;
//			const float c1hiy = boxRight.max.y * idiry - oody;
//			const float c1min = spanBeginKepler(c1lox, c1hix, c1loy, c1hiy, c1loz, c1hiz, ray.origin.w);
//			const float c1max = spanEndKepler(c1lox, c1hix, c1loy, c1hiy, c1loz, c1hiz, ray.direction.w);
//
//#endif
//
//			const bool traverseChild0 = c0max >= c0min;
//			const bool traverseChild1 = c1max >= c1min;
//
//
//			//Neither child is intersected, change the current node
//			if (!traverseChild0 && !traverseChild1)
//			{
//				currentNode = traversalStack[stackPtr--];
//			}
//			else
//			{
//				currentNode = traverseChild0 ? childL : childR;
//
//				//Both are intersected
//				if (traverseChild0 && traverseChild1)
//				{
//					if (c1min < c0min) {
//						Swap(currentNode, childR);
//					}
//
//					traversalStack[++stackPtr] = childR;
//				}
//			}
//
//			//hold the first leaf
//			if (IsLeaf(currentNode) && !IsLeaf(currentLeaf))     // Postpone leaf
//			{
//				currentLeaf = currentNode;
//				currentNode = traversalStack[stackPtr--];
//			}
//
//
//			if (!__any(!IsLeaf(currentLeaf))) {
//				break;
//			}
//
//	}
//
//		// Process postponed leaf nodes.
//
//		while (IsLeaf(currentLeaf))
//		{
//			const uint faceID = currentLeaf - leafSize;
//			const glm::uvec3 face = faces[faceID].indices;
//
//			float bary1;
//			float bary2;
//			float tTemp;
//
//			const glm::vec3 pos0 = vertices[face.x].position;
//			const glm::vec3 pos1 = vertices[face.y].position;
//			const glm::vec3 pos2 = vertices[face.z].position;
//
//			if (FindTriangleIntersect(pos0, pos1, pos2,
//				ray.origin, kx, ky, kz, Sx, Sy, Sz,
//				tTemp, ray.direction.w, bary1, bary2)) {
//
//				ray.direction.w = tTemp;
//				ray.bary = glm::vec2(bary1, bary2);
//				ray.currentHit = faceID;
//
//			}
//
//			//go through the second postponed leaf
//			currentLeaf = currentNode;
//			if (IsLeaf(currentNode))
//			{
//				currentNode = traversalStack[stackPtr--];
//			}
//		}
//
//		//cut the losses
//		if (__popc(__ballot(true)) < DYNAMIC_FETCH_THRESHOLD) {
//			break;
//		}
//}
//}