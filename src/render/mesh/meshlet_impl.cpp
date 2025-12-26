/**
 * @file meshlet_impl.cpp
 * @brief Non-module implementation file for meshoptimizer integration
 * 
 * This file exists because meshoptimizer uses C headers that are not
 * compatible with MSVC's module dependency scanning phase.
 */

#include <meshoptimizer.h>
#include <cstdint>
#include <cstring>

// Forward declarations matching meshlet.ixx types
namespace synodic::soul::mesh {

struct Meshlet {
	std::uint32_t vertexOffset;
	std::uint32_t triangleOffset;
	std::uint8_t vertexCount;
	std::uint8_t triangleCount;
	std::uint16_t padding;
};

struct MeshletBounds {
	float centerX, centerY, centerZ;
	float radius;
	float coneAxisX, coneAxisY, coneAxisZ;
	float coneCutoff;
};

struct MeshletOptions {
	std::uint32_t maxVertices;
	std::uint32_t maxTriangles;
	float coneWeight;
};

} // namespace synodic::soul::mesh

// External C-style API for meshlet generation (called from module code)
extern "C" {

std::size_t meshlet_build_meshlets_bound(
	std::size_t indexCount,
	std::size_t maxVertices,
	std::size_t maxTriangles)
{
	return meshopt_buildMeshletsBound(indexCount, maxVertices, maxTriangles);
}

std::size_t meshlet_build_meshlets(
	meshopt_Meshlet* meshlets,
	unsigned int* meshletVertices,
	unsigned char* meshletTriangles,
	const unsigned int* indices,
	std::size_t indexCount,
	const float* positions,
	std::size_t vertexCount,
	std::size_t positionStride,
	std::size_t maxVertices,
	std::size_t maxTriangles,
	float coneWeight)
{
	return meshopt_buildMeshlets(
		meshlets,
		meshletVertices,
		meshletTriangles,
		indices,
		indexCount,
		positions,
		vertexCount,
		positionStride,
		maxVertices,
		maxTriangles,
		coneWeight);
}

void meshlet_compute_bounds(
	const unsigned int* meshletVertices,
	const unsigned char* meshletTriangles,
	std::size_t triangleCount,
	const float* positions,
	std::size_t vertexCount,
	std::size_t positionStride,
	float* outCenter,
	float* outRadius,
	float* outConeAxis,
	float* outConeCutoff)
{
	meshopt_Bounds bounds = meshopt_computeMeshletBounds(
		meshletVertices,
		meshletTriangles,
		triangleCount,
		positions,
		vertexCount,
		positionStride);
	
	outCenter[0] = bounds.center[0];
	outCenter[1] = bounds.center[1];
	outCenter[2] = bounds.center[2];
	*outRadius = bounds.radius;
	outConeAxis[0] = bounds.cone_axis[0];
	outConeAxis[1] = bounds.cone_axis[1];
	outConeAxis[2] = bounds.cone_axis[2];
	*outConeCutoff = bounds.cone_cutoff;
}

// meshopt_Meshlet struct size for interop
std::size_t meshlet_meshopt_meshlet_size() {
	return sizeof(meshopt_Meshlet);
}

// Get meshlet field offsets for manual access
void meshlet_get_meshopt_meshlet(
	const meshopt_Meshlet* m,
	unsigned int* vertexOffset,
	unsigned int* triangleOffset,
	unsigned int* vertexCount,
	unsigned int* triangleCount)
{
	*vertexOffset = m->vertex_offset;
	*triangleOffset = m->triangle_offset;
	*vertexCount = m->vertex_count;
	*triangleCount = m->triangle_count;
}

} // extern "C"
