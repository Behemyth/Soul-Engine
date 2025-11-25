export module synodic.soul.render.mesh:mesh;

import std;
import synodic.soul.core;
import synodic.soul.raster;  // For GPUBufferHandle
import :vertex;

// CPU-side mesh data - owns vertex and index data before GPU upload
export struct MeshData {
	std::vector<PBRVertex> vertices;
	std::vector<Index> indices;

	[[nodiscard]] std::size_t VertexBufferSize() const noexcept {
		return vertices.size() * sizeof(PBRVertex);
	}

	[[nodiscard]] std::size_t IndexBufferSize() const noexcept {
		return indices.size() * sizeof(Index);
	}

	[[nodiscard]] std::size_t TotalSize() const noexcept {
		return VertexBufferSize() + IndexBufferSize();
	}

	[[nodiscard]] bool IsValid() const noexcept {
		return !vertices.empty() && !indices.empty();
	}

	// Compute tangents for all triangles
	void ComputeTangents() {
		for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
			ComputeTangent(
				vertices[indices[i]],
				vertices[indices[i + 1]],
				vertices[indices[i + 2]]);
		}
	}
};

// Axis-aligned bounding box
export struct AABB {
	vec3 min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	vec3 max{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};

	constexpr AABB() = default;
	constexpr AABB(vec3 min_, vec3 max_) : min(min_), max(max_) {}

	[[nodiscard]] constexpr vec3 Center() const {
		return (min + max) * 0.5f;
	}

	[[nodiscard]] constexpr vec3 Extents() const {
		return (max - min) * 0.5f;
	}

	void Expand(const vec3& point) {
		min.x = std::min(min.x, point.x);
		min.y = std::min(min.y, point.y);
		min.z = std::min(min.z, point.z);
		max.x = std::max(max.x, point.x);
		max.y = std::max(max.y, point.y);
		max.z = std::max(max.z, point.z);
	}
};

// GPU-resident mesh - holds handles to uploaded buffers
export struct GPUMesh {
	GPUBufferHandle vertexBuffer = 0;   // Handle to GPU vertex buffer
	GPUBufferHandle indexBuffer = 0;    // Handle to GPU index buffer
	std::uint32_t indexCount = 0;
	std::uint32_t vertexCount = 0;
	AABB bounds;

	[[nodiscard]] bool IsValid() const noexcept {
		return vertexBuffer != 0 && indexBuffer != 0 && indexCount > 0;
	}
};

// Submesh for multi-material meshes (glTF primitives)
export struct Submesh {
	std::uint32_t indexOffset = 0;
	std::uint32_t indexCount = 0;
	std::uint32_t materialIndex = 0;
	AABB bounds;
};

// Full mesh with submeshes
export struct Mesh {
	GPUMesh gpu;
	std::vector<Submesh> submeshes;

	// Single-submesh mesh
	[[nodiscard]] static Mesh FromGPUMesh(GPUMesh gpuMesh) {
		Mesh mesh;
		mesh.gpu = std::move(gpuMesh);
		mesh.submeshes.push_back({0, mesh.gpu.indexCount, 0, mesh.gpu.bounds});
		return mesh;
	}
};

// Compute AABB from mesh data
export inline AABB ComputeAABB(const MeshData& data) {
	AABB bounds;
	for (const auto& vertex : data.vertices) {
		bounds.Expand(vertex.position);
	}
	return bounds;
}
