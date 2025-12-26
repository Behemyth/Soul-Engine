/**
 * @file commands.ixx
 * @brief Rasterization command types for command list recording
 * 
 * Commands follow "No Graphics API" patterns where possible:
 * - GPU pointer-based root arguments for shader data
 * - Simplified barrier model (stage-only)
 * - Separated depth-stencil state
 * 
 * @see https://www.sebastianaaltonen.com/blog/no-graphics-api
 */
export module synodic.soul.raster:commands;

import std;
import synodic.soul.core;
import :types;
import :resource;
import :gpu_pointer;
import :barrier;
import :depth_stencil_state;
import :blend_state;

// Temporary type until proper buffer types are available
template<typename T> using ExternalBuffer = std::vector<T>;

// GPU buffer handle - opaque handle to a GPU-resident buffer
export using GPUBufferHandle = std::uint64_t;
export constexpr GPUBufferHandle InvalidGPUBuffer = 0;

export enum class CommandType : std::uint8_t {
	// Bindless draw commands (GPU pointer pattern)
	Draw,              ///< Basic non-indexed draw (uses bound pipeline)
	DrawIndexed,       ///< Basic indexed draw (uses bound pipeline)
	DrawWithPointers,  ///< Draw with GPU pointer root arguments (bindless)
	DrawIndirectWithPointers, ///< Multi-draw indirect with pointers (fully GPU-driven)
	
	// Mesh shader commands (VK_EXT_mesh_shader)
	DrawMeshTasks,     ///< Dispatch mesh shader workgroups
	DrawMeshTasksIndirect, ///< GPU-driven mesh shader dispatch
	
	// Compute dispatch commands
	Dispatch,          ///< Dispatch compute with GPU pointer root data
	DispatchIndirect,  ///< Indirect dispatch with GPU-generated arguments
	
	// Dynamic state commands
	BindPipeline,
	SetViewport,
	SetScissor,
	SetDepthStencilState,
	SetBlendState,
	SetTextureHeap,
	
	// Synchronization
	Barrier,
	MemoryBarrier,
	
	// Resource updates
	UpdateBuffer,
	CopyBuffer,
};

// Draw non-indexed geometry
export struct DrawCommand {
	std::uint32_t vertexCount = 0;
	std::uint32_t instanceCount = 1;
	std::uint32_t firstVertex = 0;
	std::uint32_t firstInstance = 0;
};

// Draw indexed geometry  
export struct DrawIndexedCommand {
	std::uint32_t indexCount = 0;
	std::uint32_t instanceCount = 1;
	std::uint32_t firstIndex = 0;
	std::int32_t vertexOffset = 0;  // Signed for negative offsets
	std::uint32_t firstInstance = 0;
};

// Bind a pipeline (identified by Entity/handle)
export struct BindPipelineCommand {
	Entity pipeline{};  // Pipeline entity/handle
};

// Set viewport dynamically
export struct SetViewportCommand {
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	float minDepth = 0.0f;
	float maxDepth = 1.0f;
};

// Set scissor rect dynamically
export struct SetScissorCommand {
	std::int32_t x = 0;
	std::int32_t y = 0;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

export struct UpdateBufferCommand {
	std::uint32_t offset = 0;
	ExternalBuffer<std::byte> data;
	Entity* buffer = nullptr;
};

export struct CopyBufferCommand {
	GPUBufferHandle srcBuffer = InvalidGPUBuffer;
	GPUBufferHandle dstBuffer = InvalidGPUBuffer;
	std::uint64_t srcOffset = 0;
	std::uint64_t dstOffset = 0;
	std::uint64_t size = 0;
};

// ============================================================================
// Bindless / GPU Pointer Commands (No Graphics API patterns)
// ============================================================================

/**
 * @brief Dispatch compute shader with GPU pointer root arguments
 * 
 * Follows "No Graphics API" pattern where shader data is passed via
 * a single 64-bit GPU pointer to a user-defined struct instead of
 * descriptor sets or push constants.
 * 
 * Usage:
 * @code
 * struct ComputeData {
 *     const float* input;   // GPU pointer
 *     float* output;        // GPU pointer
 *     uint32_t count;
 * };
 * GPUPointer<ComputeData> data = allocator.Allocate<ComputeData>();
 * data->input = inputBuffer.GPU();
 * data->output = outputBuffer.GPU();
 * data->count = 1024;
 * 
 * commands.DispatchWithPointer({data.GPU(), {128, 1, 1}});
 * @endcode
 */
export struct DispatchCommand {
	GPUDeviceAddress rootData = InvalidGPUAddress;  ///< GPU pointer to shader data struct
	std::uint32_t groupCountX = 1;
	std::uint32_t groupCountY = 1;
	std::uint32_t groupCountZ = 1;
	
	constexpr DispatchCommand() noexcept = default;
	
	constexpr DispatchCommand(GPUDeviceAddress data, 
	                          std::uint32_t x, std::uint32_t y = 1, std::uint32_t z = 1) noexcept
		: rootData(data), groupCountX(x), groupCountY(y), groupCountZ(z)
	{}
};

/**
 * @brief Indirect dispatch with GPU-generated arguments
 * 
 * Both root data pointer and dispatch arguments can be GPU-generated.
 */
export struct DispatchIndirectCommand {
	GPUDeviceAddress rootData = InvalidGPUAddress;  ///< GPU pointer to shader data
	GPUDeviceAddress arguments = InvalidGPUAddress; ///< GPU pointer to {groupCountX, Y, Z}
	
	constexpr DispatchIndirectCommand() noexcept = default;
	
	constexpr DispatchIndirectCommand(GPUDeviceAddress data, GPUDeviceAddress args) noexcept
		: rootData(data), arguments(args)
	{}
};

/**
 * @brief Draw with GPU pointer root arguments
 * 
 * Passes separate GPU pointers for vertex and pixel shader data,
 * following the blog's pattern of two data pointers per draw.
 * 
 * With bindless, vertex buffers are accessed via pointers in the
 * shader data struct rather than bound via BindVertexBuffer.
 * 
 * Usage:
 * @code
 * struct VertexData {
 *     const Vertex* vertices;  // GPU pointer to vertex buffer
 *     mat4 modelViewProj;
 * };
 * struct PixelData {
 *     uint32_t albedoTexture;  // Texture heap index
 *     uint32_t normalTexture;
 *     float roughness;
 * };
 * 
 * commands.DrawWithPointers({
 *     .vertexData = vertexData.GPU(),
 *     .pixelData = pixelData.GPU(),
 *     .indexBuffer = indexBuffer.GPU(),
 *     .indexCount = mesh.indexCount,
 * });
 * @endcode
 */
export struct DrawWithPointersCommand {
	GPUDeviceAddress vertexData = InvalidGPUAddress;  ///< GPU pointer to vertex shader data
	GPUDeviceAddress pixelData = InvalidGPUAddress;   ///< GPU pointer to pixel shader data
	GPUDeviceAddress indexBuffer = InvalidGPUAddress; ///< GPU pointer to index buffer (optional)
	std::uint32_t vertexCount = 0;      ///< Vertex count (if not indexed)
	std::uint32_t indexCount = 0;       ///< Index count (if indexed)
	std::uint32_t instanceCount = 1;
	std::uint32_t firstVertex = 0;
	std::uint32_t firstIndex = 0;
	std::int32_t vertexOffset = 0;      ///< Vertex offset for indexed draws
	std::uint32_t firstInstance = 0;
	bool use32BitIndices = true;
	
	constexpr DrawWithPointersCommand() noexcept = default;
	
	/**
	 * @brief Check if this is an indexed draw
	 */
	[[nodiscard]] constexpr bool IsIndexed() const noexcept {
		return indexBuffer != InvalidGPUAddress && indexCount > 0;
	}
};

/**
 * @brief Multi-draw indirect with GPU pointer root arguments
 * 
 * Enables fully GPU-driven rendering where both per-draw data pointers
 * AND draw arguments are generated by compute shaders.
 * 
 * This is the ultimate bindless pattern for culling/LOD systems.
 */
export struct DrawIndirectWithPointersCommand {
	GPUDeviceAddress vertexDataArray = InvalidGPUAddress;  ///< Array of vertex data pointers
	GPUDeviceAddress pixelDataArray = InvalidGPUAddress;   ///< Array of pixel data pointers
	std::uint32_t vertexDataStride = 0;   ///< Stride between vertex data structs
	std::uint32_t pixelDataStride = 0;    ///< Stride between pixel data structs
	GPUDeviceAddress arguments = InvalidGPUAddress;  ///< GPU pointer to draw arguments
	GPUDeviceAddress drawCount = InvalidGPUAddress;  ///< GPU pointer to draw count (optional)
	std::uint32_t maxDrawCount = 0;       ///< Maximum draws (if drawCount is GPU-generated)
	std::uint32_t argumentStride = 0;     ///< Stride between draw argument structs
	bool indexed = true;
	
	constexpr DrawIndirectWithPointersCommand() noexcept = default;
};

/**
 * @brief Set active texture heap for bindless texture access
 * 
 * Must be called before draws/dispatches that use bindless textures.
 * The heap GPU address is used by shaders to index into the texture array.
 */
export struct SetTextureHeapCommand {
	GPUDeviceAddress textureHeap = InvalidGPUAddress;
	GPUDeviceAddress samplerHeap = InvalidGPUAddress;  ///< Optional separate sampler heap
	
	constexpr SetTextureHeapCommand() noexcept = default;
	
	constexpr explicit SetTextureHeapCommand(GPUDeviceAddress textures, 
	                                         GPUDeviceAddress samplers = InvalidGPUAddress) noexcept
		: textureHeap(textures), samplerHeap(samplers)
	{}
};

// ============================================================================
// Mesh Shader Commands (VK_EXT_mesh_shader)
// ============================================================================

/**
 * @brief Dispatch mesh shader workgroups with GPU pointer root arguments
 * 
 * Modern replacement for traditional vertex+index draw calls.
 * Uses mesh shaders for meshlet-based geometry processing.
 * 
 * Usage:
 * @code
 * struct MeshShaderData {
 *     const MeshVertex* vertices;   // GPU pointer to vertex buffer
 *     const uint32_t* meshletVertices;  // Meshlet vertex indices
 *     const uint32_t* meshletTriangles; // Packed triangle indices
 *     const Meshlet* meshlets;      // Meshlet descriptors
 *     mat4 mvp;
 *     uint32_t meshletCount;
 * };
 * 
 * commands.DrawMeshTasks({
 *     .meshData = meshData.GPU(),
 *     .pixelData = pixelData.GPU(),
 *     .groupCountX = meshletCount,  // One workgroup per meshlet
 *     .groupCountY = 1,
 *     .groupCountZ = 1,
 * });
 * @endcode
 */
export struct DrawMeshTasksCommand {
	GPUDeviceAddress meshData = InvalidGPUAddress;   ///< GPU pointer to mesh shader data
	GPUDeviceAddress pixelData = InvalidGPUAddress;  ///< GPU pointer to pixel shader data
	std::uint32_t groupCountX = 1;  ///< Number of mesh shader workgroups X
	std::uint32_t groupCountY = 1;  ///< Number of mesh shader workgroups Y
	std::uint32_t groupCountZ = 1;  ///< Number of mesh shader workgroups Z
	
	constexpr DrawMeshTasksCommand() noexcept = default;
	
	constexpr DrawMeshTasksCommand(GPUDeviceAddress mesh, GPUDeviceAddress pixel,
	                               std::uint32_t x, std::uint32_t y = 1, std::uint32_t z = 1) noexcept
		: meshData(mesh), pixelData(pixel), groupCountX(x), groupCountY(y), groupCountZ(z)
	{}
};

/**
 * @brief Indirect mesh shader dispatch with GPU-generated arguments
 * 
 * Enables fully GPU-driven meshlet rendering where dispatch counts
 * are generated by compute/task shaders (e.g., after culling).
 */
export struct DrawMeshTasksIndirectCommand {
	GPUDeviceAddress meshData = InvalidGPUAddress;   ///< GPU pointer to mesh shader data
	GPUDeviceAddress pixelData = InvalidGPUAddress;  ///< GPU pointer to pixel shader data
	GPUDeviceAddress arguments = InvalidGPUAddress;  ///< GPU pointer to {groupCountX, Y, Z}
	GPUDeviceAddress drawCount = InvalidGPUAddress;  ///< GPU pointer to draw count (optional)
	std::uint32_t maxDrawCount = 1;    ///< Maximum draws if drawCount is GPU-generated
	std::uint32_t argumentStride = 12; ///< Stride between argument structs (3 * uint32 = 12)
	
	constexpr DrawMeshTasksIndirectCommand() noexcept = default;
};

// Re-export barrier, depth-stencil, and blend commands from their modules
export using ::BarrierCommand;
export using ::MemoryBarrierCommand;
export using ::SetDepthStencilStateCommand;
export using ::SetBlendStateCommand;