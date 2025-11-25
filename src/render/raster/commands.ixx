export module synodic.soul.raster:commands;

import std;
import synodic.soul.core;
import :types;
import :resource;

// Temporary type until proper buffer types are available
template<typename T> using ExternalBuffer = std::vector<T>;

// GPU buffer handle - opaque handle to a GPU-resident buffer
export using GPUBufferHandle = std::uint64_t;
export constexpr GPUBufferHandle InvalidGPUBuffer = 0;

export enum class CommandType : std::uint8_t {
	Draw,
	DrawIndexed,
	DrawIndirect,
	BindVertexBuffer,
	BindIndexBuffer,
	SetPushConstants,
	BindPipeline,
	SetViewport,
	SetScissor,
	UpdateBuffer,
	UpdateTexture,
	CopyBuffer,
	CopyTexture
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

// Bind vertex buffer to a binding slot
export struct BindVertexBufferCommand {
	GPUBufferHandle buffer = InvalidGPUBuffer;
	std::uint32_t binding = 0;
	std::uint64_t offset = 0;
};

// Bind index buffer
export struct BindIndexBufferCommand {
	GPUBufferHandle buffer = InvalidGPUBuffer;
	std::uint64_t offset = 0;
	bool use32BitIndices = true;  // false = 16-bit indices
};

// Set push constant data
export struct SetPushConstantsCommand {
	std::array<std::byte, 128> data{};  // Max 128 bytes (typical limit)
	std::uint32_t size = 0;
	std::uint32_t offset = 0;
	
	// Helper to set typed data
	template<typename T>
	void Set(const T& value, std::uint32_t offsetBytes = 0) {
		static_assert(sizeof(T) <= 128, "Push constant data exceeds 128 bytes");
		std::memcpy(data.data() + offsetBytes, &value, sizeof(T));
		size = sizeof(T) + offsetBytes;
		offset = 0;
	}
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

export struct DrawIndirectCommand {
	GPUBufferHandle buffer = InvalidGPUBuffer;
	std::uint64_t offset = 0;
	std::uint32_t drawCount = 1;
	std::uint32_t stride = 0;
};

export struct UpdateBufferCommand {
	std::uint32_t offset = 0;
	ExternalBuffer<std::byte> data;
	Entity* buffer = nullptr;
};

export struct UpdateTextureCommand {
};

export struct CopyBufferCommand {
	GPUBufferHandle srcBuffer = InvalidGPUBuffer;
	GPUBufferHandle dstBuffer = InvalidGPUBuffer;
	std::uint64_t srcOffset = 0;
	std::uint64_t dstOffset = 0;
	std::uint64_t size = 0;
};

export struct CopyTextureCommand {
};
