export module synodic.soul.render.mesh:mesh;

import std;
import synodic.library;
import synodic.soul.core;
import synodic.soul.raster;  // For GPUBufferHandle
import :vertex;

// Use synodic library's AABB with min/max storage
export using AABB = synodic::math::AABB<synodic::math::AABBMinMax<float>>;

// CPU-side mesh data - raw interleaved vertex buffer with layout metadata
export struct MeshData {
	std::vector<float> vertexData;    // Raw interleaved vertex attributes
	std::vector<Index> indices;
	VertexLayout layout;              // Describes how to interpret vertexData
	std::uint32_t vertexCount = 0;

	[[nodiscard]] std::size_t VertexBufferSize() const noexcept {
		return vertexData.size() * sizeof(float);
	}

	[[nodiscard]] std::size_t IndexBufferSize() const noexcept {
		return indices.size() * sizeof(Index);
	}

	[[nodiscard]] std::size_t TotalSize() const noexcept {
		return VertexBufferSize() + IndexBufferSize();
	}

	[[nodiscard]] bool IsValid() const noexcept {
		return !vertexData.empty() && !indices.empty() && vertexCount > 0;
	}

	// Get pointer to a specific vertex's data
	[[nodiscard]] const float* VertexAt(std::size_t index) const noexcept {
		return vertexData.data() + (index * layout.stride / sizeof(float));
	}

	[[nodiscard]] float* VertexAt(std::size_t index) noexcept {
		return vertexData.data() + (index * layout.stride / sizeof(float));
	}

	// Reserve space for N vertices with the current layout
	void Reserve(std::size_t count) {
		vertexData.reserve(count * layout.stride / sizeof(float));
		vertexCount = 0;
	}

	// Resize to hold exactly N vertices
	void Resize(std::size_t count) {
		vertexData.resize(count * layout.stride / sizeof(float));
		vertexCount = static_cast<std::uint32_t>(count);
	}

	// TODO: Implement ComputeTangents
	void ComputeTangents() {
		// Placeholder - requires knowing attribute offsets from layout
	}
};

// GPU-resident mesh - holds handles to uploaded buffers
export struct GPUMesh {
	GPUBufferHandle vertexBuffer = 0;   // Handle to GPU vertex buffer
	GPUBufferHandle indexBuffer = 0;    // Handle to GPU index buffer
	std::uint32_t indexCount = 0;
	std::uint32_t vertexCount = 0;
	VertexLayout layout;                // Layout for pipeline binding
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

// Compute AABB from mesh data (assumes position is first attribute, vec3)
export inline AABB ComputeAABB(const MeshData& data) {
	AABB bounds = AABB::Empty();
	const std::size_t floatsPerVertex = data.layout.stride / sizeof(float);

	for (std::uint32_t i = 0; i < data.vertexCount; ++i) {
		const float* vertex = data.vertexData.data() + (i * floatsPerVertex);
		bounds = synodic::math::Expand(bounds, synodic::math::vec3{vertex[0], vertex[1], vertex[2]});
	}
	return bounds;
}
