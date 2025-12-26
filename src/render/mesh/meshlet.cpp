/**
 * @file meshlet.cpp
 * @brief Meshlet generation implementation - module interface
 * 
 * Note: The actual meshoptimizer calls are in meshlet_impl.cpp (non-module)
 * because meshoptimizer's headers use C includes that aren't compatible
 * with MSVC's module dependency scanning.
 */

module synodic.soul.render.mesh;

import :meshlet;
import std;

// External C functions from meshlet_impl.cpp
extern "C" {
	std::size_t meshlet_build_meshlets_bound(
		std::size_t indexCount,
		std::size_t maxVertices,
		std::size_t maxTriangles);
	
	std::size_t meshlet_build_meshlets(
		void* meshlets,  // meshopt_Meshlet*
		unsigned int* meshletVertices,
		unsigned char* meshletTriangles,
		const unsigned int* indices,
		std::size_t indexCount,
		const float* positions,
		std::size_t vertexCount,
		std::size_t positionStride,
		std::size_t maxVertices,
		std::size_t maxTriangles,
		float coneWeight);
	
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
		float* outConeCutoff);
	
	std::size_t meshlet_meshopt_meshlet_size();
	
	void meshlet_get_meshopt_meshlet(
		const void* m,
		unsigned int* vertexOffset,
		unsigned int* triangleOffset,
		unsigned int* vertexCount,
		unsigned int* triangleCount);
}

namespace synodic::soul::mesh
{

MeshletData GenerateMeshlets(
	const MeshData& mesh,
	const MeshletOptions& options)
{
	if (!mesh.IsValid()) {
		return {};
	}
	
	// Find position attribute offset in vertex layout
	const float* positions = mesh.vertexData.data();
	std::size_t positionStride = mesh.layout.stride;
	
	return GenerateMeshletsRaw(
		positions,
		positionStride,
		mesh.vertexCount,
		mesh.indices.data(),
		mesh.indices.size(),
		options
	);
}

MeshletData GenerateMeshletsRaw(
	const float* positions,
	std::size_t positionStride,
	std::size_t vertexCount,
	const Index* indices,
	std::size_t indexCount,
	const MeshletOptions& options)
{
	if (!positions || !indices || vertexCount == 0 || indexCount == 0) {
		return {};
	}
	
	// Calculate maximum possible meshlet count using external C function
	const std::size_t maxMeshlets = meshlet_build_meshlets_bound(
		indexCount, 
		options.maxVertices, 
		options.maxTriangles
	);
	
	// Allocate output structures
	// meshopt_Meshlet is 16 bytes: { vertex_offset, triangle_offset, vertex_count, triangle_count }
	std::vector<std::byte> meshoptMeshletStorage(maxMeshlets * meshlet_meshopt_meshlet_size());
	std::vector<unsigned int> meshletVertices(maxMeshlets * options.maxVertices);
	std::vector<unsigned char> meshletTriangles(maxMeshlets * options.maxTriangles * 3);
	
	// Build meshlets using external C function
	std::size_t meshletCount = meshlet_build_meshlets(
		meshoptMeshletStorage.data(),
		meshletVertices.data(),
		meshletTriangles.data(),
		indices,
		indexCount,
		positions,
		vertexCount,
		positionStride,
		options.maxVertices,
		options.maxTriangles,
		options.coneWeight
	);
	
	if (meshletCount == 0) {
		return {};
	}
	
	// Get the last meshlet's info to calculate total sizes
	std::size_t meshletStructSize = meshlet_meshopt_meshlet_size();
	unsigned int lastVertexOffset, lastTriangleOffset, lastVertexCount, lastTriangleCount;
	meshlet_get_meshopt_meshlet(
		meshoptMeshletStorage.data() + (meshletCount - 1) * meshletStructSize,
		&lastVertexOffset, &lastTriangleOffset, &lastVertexCount, &lastTriangleCount);
	
	std::size_t totalVertices = lastVertexOffset + lastVertexCount;
	std::size_t totalTriangles = lastTriangleOffset + ((lastTriangleCount * 3 + 3) & ~3u);
	
	// Build output structure
	MeshletData result;
	result.meshlets.reserve(meshletCount);
	result.bounds.reserve(meshletCount);
	result.vertexIndices.resize(totalVertices);
	result.primitiveIndices.resize(totalTriangles);
	
	// Copy vertex indices
	std::memcpy(result.vertexIndices.data(), meshletVertices.data(), 
		totalVertices * sizeof(std::uint32_t));
	
	// Copy primitive indices
	std::memcpy(result.primitiveIndices.data(), meshletTriangles.data(), totalTriangles);
	
	// Convert meshlets and compute bounds
	for (std::size_t i = 0; i < meshletCount; ++i) {
		// Get meshopt_Meshlet fields via external function
		unsigned int srcVertexOffset, srcTriangleOffset, srcVertexCount, srcTriangleCount;
		meshlet_get_meshopt_meshlet(
			meshoptMeshletStorage.data() + i * meshletStructSize,
			&srcVertexOffset, &srcTriangleOffset, &srcVertexCount, &srcTriangleCount);
		
		// Convert meshlet descriptor
		Meshlet dst;
		dst.vertexOffset = srcVertexOffset;
		dst.triangleOffset = srcTriangleOffset;
		dst.vertexCount = static_cast<std::uint8_t>(srcVertexCount);
		dst.triangleCount = static_cast<std::uint8_t>(srcTriangleCount);
		dst.padding = 0;
		result.meshlets.push_back(dst);
		
		// Compute bounds using external C function
		float center[3], radius, coneAxis[3], coneCutoff;
		meshlet_compute_bounds(
			&meshletVertices[srcVertexOffset],
			&meshletTriangles[srcTriangleOffset],
			srcTriangleCount,
			positions,
			vertexCount,
			positionStride,
			center, &radius, coneAxis, &coneCutoff);
		
		// Convert bounds
		MeshletBounds bounds;
		bounds.centerX = center[0];
		bounds.centerY = center[1];
		bounds.centerZ = center[2];
		bounds.radius = radius;
		bounds.coneAxisX = coneAxis[0];
		bounds.coneAxisY = coneAxis[1];
		bounds.coneAxisZ = coneAxis[2];
		bounds.coneCutoff = coneCutoff;
		result.bounds.push_back(bounds);
	}
	
	return result;
}

} // namespace synodic::soul::mesh
