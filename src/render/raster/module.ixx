export module synodic.soul.raster;

import std;
import synodic.periapsis;

import synodic.soul.core;

export import :types;
export import :resource;
export import :gpu_pointer;
export import :frame_allocator;
export import :barrier;
export import :texture_heap;
export import :depth_stencil_state;
export import :blend_state;
export import :commands;
export import :device;
export import :command_list;

// Flags for pass execution control
export enum class PassExecutionFlags : std::uint32_t {
	None = 0,
	FirstPass = 1 << 0,    // First pass in frame - acquire swapchain image
	LastPass = 1 << 1,     // Last pass in frame - signal presentation semaphores
	SinglePass = FirstPass | LastPass,  // Only one pass in frame
};

export constexpr PassExecutionFlags operator|(PassExecutionFlags a, PassExecutionFlags b) {
	return static_cast<PassExecutionFlags>(
		static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

export constexpr bool HasFlag(PassExecutionFlags flags, PassExecutionFlags flag) {
	return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(flag)) != 0;
}

// Buffer usage flags for GPU buffers
export enum class BufferUsage : std::uint32_t {
	None = 0,
	Vertex = 1 << 0,              // Can be bound as vertex buffer
	Index = 1 << 1,               // Can be bound as index buffer
	Uniform = 1 << 2,             // Can be bound as uniform buffer
	Storage = 1 << 3,             // Can be bound as storage buffer
	TransferSrc = 1 << 4,         // Source for transfer operations
	TransferDst = 1 << 5,         // Destination for transfer operations
	ShaderDeviceAddress = 1 << 6, // Can be accessed via GPU pointer (BDA)
};

export constexpr BufferUsage operator|(BufferUsage a, BufferUsage b) {
	return static_cast<BufferUsage>(
		static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

export constexpr bool HasBufferUsage(BufferUsage flags, BufferUsage flag) {
	return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(flag)) != 0;
}

// Buffer memory location preference
export enum class BufferMemory {
	DeviceLocal,     // Fast GPU memory, requires staging for upload
	HostVisible,     // CPU-accessible, good for dynamic updates
	HostCached,      // CPU-cached, good for readback
};

// Buffer creation parameters
export struct BufferDesc {
	std::size_t size = 0;
	BufferUsage usage = BufferUsage::None;
	BufferMemory memory = BufferMemory::DeviceLocal;
};

export class RasterModule
{
public:
	RasterModule() = default;
	virtual ~RasterModule() = default;

	RasterModule(const RasterModule&)	  = delete;
	RasterModule(RasterModule&&) noexcept = default;

	RasterModule& operator=(const RasterModule&)	 = delete;
	RasterModule& operator=(RasterModule&&) noexcept = default;

	virtual void Present() = 0;

	virtual Entity CreatePass(const ShaderSet&, std::function<void(Entity)>)			= 0;
	virtual Entity CreateSubPass(Entity, const ShaderSet&, std::function<void(Entity)>) = 0;
	
	// Legacy single-pass execution (calls ExecutePassWithFlags with SinglePass)
	virtual void ExecutePass(Entity, Entity, CommandList&)								= 0;
	
	// Multi-pass execution with explicit control over synchronization
	virtual void ExecutePassWithFlags(Entity passEntity, Entity surfaceEntity, 
		CommandList& commands, PassExecutionFlags flags) = 0;

	virtual void CreatePassInput(Entity, Entity, Format)  = 0;
	virtual void CreatePassOutput(Entity, Entity, Format) = 0;

	virtual Entity CreateSurface(NativeSurfaceHandle, peri::math::uvec2) = 0;
	virtual void UpdateSurface(Entity, peri::math::uvec2)	  = 0;
	virtual void RemoveSurface(Entity)			  = 0;
	virtual void AttachSurface(Entity, Entity)	  = 0;
	virtual void DetachSurface(Entity, Entity)	  = 0;

	// Buffer management
	virtual GPUBufferHandle CreateBuffer(const BufferDesc& desc) = 0;
	virtual void DestroyBuffer(GPUBufferHandle handle) = 0;
	
	// Get GPU device address for a buffer (returns InvalidGPUAddress if not created with ShaderDeviceAddress)
	virtual GPUDeviceAddress GetBufferGPUAddress(GPUBufferHandle handle) = 0;
	
	// Upload data to a device-local buffer (uses staging + transfer queue)
	virtual void UploadBufferData(GPUBufferHandle handle, const void* data, 
		std::size_t size, std::size_t offset = 0) = 0;
	
	// Map/unmap for host-visible buffers
	virtual void* MapBuffer(GPUBufferHandle handle) = 0;
	virtual void UnmapBuffer(GPUBufferHandle handle) = 0;
	virtual void FlushBuffer(GPUBufferHandle handle, std::size_t offset = 0, 
		std::size_t size = std::numeric_limits<std::size_t>::max()) = 0;

	// Bindless allocator (No Graphics API pattern)
	// Returns per-frame allocator for GPU data passed via root arguments
	virtual FrameAllocator* GetFrameAllocator() = 0;
	
	// Reset frame allocator (call at start of each frame)
	virtual void ResetFrameAllocator() = 0;

	// Agnostic raster API interface
	virtual void Compile(CommandList&) = 0;
};
