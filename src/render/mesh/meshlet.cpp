/**
 * @file meshlet.cpp
 * @brief Meshlet generation implementation using meshoptimizer
 */
module synodic.soul.render.mesh;

import :meshlet;
import std;

// meshoptimizer headers - non-modular C library
#include <meshoptimizer.h>

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
	
	// Calculate maximum possible meshlet count
	const std::size_t maxMeshlets = meshopt_buildMeshletsBound(
		indexCount, 
		options.maxVertices, 
		options.maxTriangles
	);
	
	// Allocate meshoptimizer output structures
	std::vector<meshopt_Meshlet> meshoptMeshlets(maxMeshlets);
	std::vector<unsigned int> meshletVertices(maxMeshlets * options.maxVertices);
	std::vector<unsigned char> meshletTriangles(maxMeshlets * options.maxTriangles * 3);
	
	// Build meshlets using meshoptimizer
	// Indices are 32-bit unsigned, positions need casting for stride
	std::size_t meshletCount = meshopt_buildMeshlets(
		meshoptMeshlets.data(),
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
	
	// Trim to actual size
	meshoptMeshlets.resize(meshletCount);
	
	// Calculate total sizes needed
	const auto& lastMeshlet = meshoptMeshlets.back();
	std::size_t totalVertices = lastMeshlet.vertex_offset + lastMeshlet.vertex_count;
	std::size_t totalTriangles = lastMeshlet.triangle_offset + 
		((lastMeshlet.triangle_count * 3 + 3) & ~3); // Aligned
	
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
	std::memcpy(result.primitiveIndices.data(), meshletTriangles.data(),
		totalTriangles);
	
	// Convert meshlets and compute bounds
	for (std::size_t i = 0; i < meshletCount; ++i) {
		const auto& src = meshoptMeshlets[i];
		
		// Convert meshlet descriptor
		Meshlet dst;
		dst.vertexOffset = src.vertex_offset;
		dst.triangleOffset = src.triangle_offset;
		dst.vertexCount = static_cast<std::uint8_t>(src.vertex_count);
		dst.triangleCount = static_cast<std::uint8_t>(src.triangle_count);
		dst.padding = 0;
		result.meshlets.push_back(dst);
		
		// Compute bounds using meshoptimizer
		meshopt_Bounds meshoptBounds = meshopt_computeMeshletBounds(
			&meshletVertices[src.vertex_offset],
			&meshletTriangles[src.triangle_offset],
			src.triangle_count,
			positions,
			vertexCount,
			positionStride
		);
		
		// Convert bounds
		MeshletBounds bounds;
		bounds.centerX = meshoptBounds.center[0];
		bounds.centerY = meshoptBounds.center[1];
		bounds.centerZ = meshoptBounds.center[2];
		bounds.radius = meshoptBounds.radius;
		bounds.coneAxisX = meshoptBounds.cone_axis[0];
		bounds.coneAxisY = meshoptBounds.cone_axis[1];
		bounds.coneAxisZ = meshoptBounds.cone_axis[2];
		bounds.coneCutoff = meshoptBounds.cone_cutoff;
		result.bounds.push_back(bounds);
	}
	
	return result;
}

} // namespace synodic::soul::mesh
