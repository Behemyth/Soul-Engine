/**
 * @file meshlet.ixx
 * @brief Meshlet data structures and generation using meshoptimizer
 * 
 * Meshlets are small clusters of vertices and triangles that can be
 * efficiently processed by mesh shaders. Each meshlet contains:
 * - Up to 64 vertices (hardware optimal)
 * - Up to 124 primitives (triangles) - hardware limit varies
 * - Bounding data for GPU culling
 */
export module synodic.soul.render.mesh:meshlet;

import std;
import synodic.periapsis;
import synodic.soul.core;
import synodic.soul.raster;
import :mesh;
import :vertex;

export namespace synodic::soul::mesh
{

// Must match shader constants
inline constexpr std::uint32_t MaxVerticesPerMeshlet = 64;
inline constexpr std::uint32_t MaxPrimitivesPerMeshlet = 124;

/**
 * @brief GPU-friendly meshlet descriptor
 * 
 * References data in separate vertex/primitive index buffers.
 * Uses 32-bit offsets to support large meshes.
 */
struct Meshlet {
	std::uint32_t vertexOffset;      // Offset into meshlet vertex buffer
	std::uint32_t triangleOffset;    // Offset into meshlet index buffer (byte offset / 4)
	std::uint8_t vertexCount;        // Number of vertices in this meshlet
	std::uint8_t triangleCount;      // Number of triangles in this meshlet
	std::uint16_t padding;           // Padding for alignment
};

static_assert(sizeof(Meshlet) == 12, "Meshlet must be 12 bytes");
static_assert(alignof(Meshlet) == 4, "Meshlet must be 4-byte aligned");

/**
 * @brief Bounding data for meshlet culling
 * 
 * Contains both sphere bounds (for frustum culling) and
 * cone bounds (for backface culling of entire meshlet).
 */
struct MeshletBounds {
	// Bounding sphere center
	float centerX;
	float centerY;
	float centerZ;
	float radius;
	
	// Normal cone for backface culling
	// Cone apex is at bounding sphere center
	// If dot(normalize(apex - cameraPos), coneAxis) > coneCutoff, meshlet is backfacing
	float coneAxisX;  // Normalized cone axis (average normal direction)
	float coneAxisY;
	float coneAxisZ;
	float coneCutoff; // cos(cone_half_angle), -1 means no culling possible
};

static_assert(sizeof(MeshletBounds) == 32, "MeshletBounds must be 32 bytes");

/**
 * @brief Complete meshlet data for a mesh
 * 
 * Contains all data needed to render mesh with mesh shaders.
 */
struct MeshletData {
	std::vector<Meshlet> meshlets;          // Meshlet descriptors
	std::vector<MeshletBounds> bounds;      // Per-meshlet culling data
	std::vector<std::uint32_t> vertexIndices;    // Indices into original vertex buffer
	std::vector<std::uint8_t> primitiveIndices;  // Local triangle indices (3 per triangle)
	
	[[nodiscard]] bool IsValid() const noexcept {
		return !meshlets.empty() && meshlets.size() == bounds.size();
	}
	
	[[nodiscard]] std::size_t MeshletCount() const noexcept {
		return meshlets.size();
	}
	
	[[nodiscard]] std::size_t TotalVertexIndices() const noexcept {
		return vertexIndices.size();
	}
	
	[[nodiscard]] std::size_t TotalPrimitiveIndices() const noexcept {
		return primitiveIndices.size();
	}
	
	// Size calculations for GPU buffer allocation
	[[nodiscard]] std::size_t MeshletBufferSize() const noexcept {
		return meshlets.size() * sizeof(Meshlet);
	}
	
	[[nodiscard]] std::size_t BoundsBufferSize() const noexcept {
		return bounds.size() * sizeof(MeshletBounds);
	}
	
	[[nodiscard]] std::size_t VertexIndexBufferSize() const noexcept {
		return vertexIndices.size() * sizeof(std::uint32_t);
	}
	
	[[nodiscard]] std::size_t PrimitiveIndexBufferSize() const noexcept {
		// Round up to 4-byte alignment for GPU
		return ((primitiveIndices.size() + 3) / 4) * 4;
	}
	
	[[nodiscard]] std::size_t TotalGPUSize() const noexcept {
		return MeshletBufferSize() + BoundsBufferSize() + 
		       VertexIndexBufferSize() + PrimitiveIndexBufferSize();
	}
};

/**
 * @brief GPU-resident meshlet mesh
 * 
 * Holds handles to uploaded meshlet buffers for mesh shader rendering.
 */
struct GPUMeshletMesh {
	// Original vertex data (still needed - mesh shader reads vertices by index)
	GPUBufferHandle vertexBuffer = 0;
	GPUDeviceAddress vertexBufferGPU = InvalidGPUAddress;
	VertexLayout vertexLayout;
	std::uint32_t vertexCount = 0;
	
	// Meshlet data
	GPUBufferHandle meshletBuffer = 0;
	GPUDeviceAddress meshletBufferGPU = InvalidGPUAddress;
	
	GPUBufferHandle boundsBuffer = 0;
	GPUDeviceAddress boundsBufferGPU = InvalidGPUAddress;
	
	GPUBufferHandle vertexIndexBuffer = 0;
	GPUDeviceAddress vertexIndexBufferGPU = InvalidGPUAddress;
	
	GPUBufferHandle primitiveIndexBuffer = 0;
	GPUDeviceAddress primitiveIndexBufferGPU = InvalidGPUAddress;
	
	std::uint32_t meshletCount = 0;
	AABB bounds;
	
	[[nodiscard]] bool IsValid() const noexcept {
		return meshletBuffer != 0 && meshletCount > 0 && vertexBuffer != 0;
	}
};

/**
 * @brief Options for meshlet generation
 */
struct MeshletOptions {
	std::uint32_t maxVertices = MaxVerticesPerMeshlet;
	std::uint32_t maxTriangles = MaxPrimitivesPerMeshlet;
	float coneWeight = 0.5f;  // Balance between size and culling efficiency (0-1)
};

/**
 * @brief Generate meshlets from mesh data
 * 
 * Uses meshoptimizer to cluster triangles into meshlets with:
 * - Optimal vertex reuse within meshlet
 * - Good spatial locality for culling
 * - Normal cone for backface culling
 * 
 * @param mesh Source mesh data (vertices must include positions as first 3 floats)
 * @param options Generation parameters
 * @return Generated meshlet data
 */
[[nodiscard]] MeshletData GenerateMeshlets(
	const MeshData& mesh,
	const MeshletOptions& options = {});

/**
 * @brief Generate meshlets from raw vertex/index data
 * 
 * Lower-level function for custom vertex formats.
 * 
 * @param positions Pointer to position data (3 floats per vertex)
 * @param positionStride Byte stride between positions
 * @param vertexCount Number of vertices
 * @param indices Triangle indices
 * @param indexCount Number of indices (must be multiple of 3)
 * @param options Generation parameters
 * @return Generated meshlet data
 */
[[nodiscard]] MeshletData GenerateMeshletsRaw(
	const float* positions,
	std::size_t positionStride,
	std::size_t vertexCount,
	const Index* indices,
	std::size_t indexCount,
	const MeshletOptions& options = {});

} // namespace synodic::soul::mesh
