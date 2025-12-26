/**
 * @file barrier.ixx
 * @brief Simplified barrier API following "No Graphics API" patterns
 * 
 * Implements stage-only barriers with hazard flags instead of per-resource
 * tracking. Modern GPUs with coherent caches and unified memory make
 * fine-grained resource barriers largely unnecessary.
 * 
 * Key simplifications:
 * - No resource lists in barrier calls
 * - Stage flags define pipeline synchronization points
 * - Hazard flags handle special cache coherency cases
 * 
 * @see https://www.sebastianaaltonen.com/blog/no-graphics-api
 */
export module synodic.soul.raster:barrier;

import std;

/**
 * @brief Pipeline stage flags for barrier synchronization
 * 
 * Defines execution stages for barrier source/destination.
 * Based on Vulkan's VkPipelineStageFlagBits2 but simplified
 * to common usage patterns.
 */
export enum class StageFlags : std::uint32_t {
	None = 0,
	
	// === Compute stages ===
	Compute             = 1 << 0,   ///< Compute shader execution
	
	// === Transfer stages ===
	Transfer            = 1 << 1,   ///< Copy/blit operations
	
	// === Raster stages ===
	VertexInput         = 1 << 2,   ///< Vertex/index buffer reads
	VertexShader        = 1 << 3,   ///< Vertex shader execution
	TessellationControl = 1 << 4,   ///< Tessellation control shader
	TessellationEval    = 1 << 5,   ///< Tessellation evaluation shader
	GeometryShader      = 1 << 6,   ///< Geometry shader execution
	FragmentShader      = 1 << 7,   ///< Fragment/pixel shader execution
	EarlyFragmentTests  = 1 << 8,   ///< Early depth/stencil tests
	LateFragmentTests   = 1 << 9,   ///< Late depth/stencil tests
	ColorAttachmentOut  = 1 << 10,  ///< Color attachment writes
	DepthStencilOut     = 1 << 11,  ///< Depth/stencil attachment writes
	
	// === Raytracing stages ===
	RayTracingShader    = 1 << 12,  ///< Any raytracing shader
	AccelerationBuild   = 1 << 13,  ///< Acceleration structure builds
	
	// === Host stages ===
	Host                = 1 << 14,  ///< CPU read/write operations
	
	// === Composite masks ===
	AllGraphics         = VertexInput | VertexShader | TessellationControl | 
	                      TessellationEval | GeometryShader | FragmentShader |
	                      EarlyFragmentTests | LateFragmentTests | 
	                      ColorAttachmentOut | DepthStencilOut,
	                      
	AllCommands         = Compute | Transfer | AllGraphics | RayTracingShader | AccelerationBuild,
	
	// === Common patterns ===
	/// Typical compute-to-compute UAV barrier
	ComputeReadWrite    = Compute,
	
	/// After render target write, before shader read
	RenderTargetToShaderRead = ColorAttachmentOut | DepthStencilOut,
	
	/// After shader write, before render target use
	ShaderWriteToRenderTarget = FragmentShader | Compute,
};

/**
 * @brief Enable bitwise operations on StageFlags
 */
export constexpr StageFlags operator|(StageFlags a, StageFlags b) noexcept {
	return static_cast<StageFlags>(
		static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

export constexpr StageFlags operator&(StageFlags a, StageFlags b) noexcept {
	return static_cast<StageFlags>(
		static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

export constexpr StageFlags operator~(StageFlags a) noexcept {
	return static_cast<StageFlags>(~static_cast<std::uint32_t>(a));
}

export constexpr StageFlags& operator|=(StageFlags& a, StageFlags b) noexcept {
	return a = a | b;
}

export constexpr StageFlags& operator&=(StageFlags& a, StageFlags b) noexcept {
	return a = a & b;
}

export constexpr bool HasStage(StageFlags flags, StageFlags stage) noexcept {
	return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(stage)) != 0;
}

/**
 * @brief Special cache hazard flags for non-standard synchronization
 * 
 * Most modern GPUs have coherent L2 caches, making explicit cache
 * management unnecessary. These flags handle edge cases where
 * special cache operations are required.
 */
export enum class HazardFlags : std::uint32_t {
	None = 0,
	
	/**
	 * @brief Descriptor/bindless heap was written
	 * 
	 * Required when CPU or GPU writes to the global descriptor heap
	 * and subsequent draws/dispatches will read those descriptors.
	 * Maps to VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT.
	 */
	Descriptors         = 1 << 0,
	
	/**
	 * @brief Indirect draw/dispatch arguments were written
	 * 
	 * Required when GPU writes indirect command buffers and
	 * subsequent indirect draws/dispatches will consume them.
	 * Maps to VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT.
	 */
	DrawArguments       = 1 << 1,
	
	/**
	 * @brief Index buffer was written
	 * 
	 * Required when GPU writes to a buffer that will be used
	 * as an index buffer in subsequent draws.
	 */
	IndexBuffer         = 1 << 2,
	
	/**
	 * @brief Conditional rendering predicate was written
	 * 
	 * Required when GPU writes conditional rendering predicates.
	 */
	ConditionalRendering = 1 << 3,
	
	/**
	 * @brief Acceleration structure was written
	 * 
	 * Required after building/updating acceleration structures
	 * before raytracing operations.
	 */
	AccelerationStructure = 1 << 4,
	
	/**
	 * @brief Shader binding table was written
	 * 
	 * Required when updating shader binding tables for raytracing.
	 */
	ShaderBindingTable   = 1 << 5,
};

/**
 * @brief Enable bitwise operations on HazardFlags
 */
export constexpr HazardFlags operator|(HazardFlags a, HazardFlags b) noexcept {
	return static_cast<HazardFlags>(
		static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

export constexpr HazardFlags operator&(HazardFlags a, HazardFlags b) noexcept {
	return static_cast<HazardFlags>(
		static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

export constexpr HazardFlags& operator|=(HazardFlags& a, HazardFlags b) noexcept {
	return a = a | b;
}

export constexpr bool HasHazard(HazardFlags flags, HazardFlags hazard) noexcept {
	return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(hazard)) != 0;
}

/**
 * @brief Barrier command for command list recording
 * 
 * Simplified barrier that only specifies stage dependencies and
 * optional hazard flags. No resource tracking required.
 * 
 * Usage:
 * @code
 * // Standard compute UAV barrier
 * commands.Barrier({StageFlags::Compute, StageFlags::Compute});
 * 
 * // Descriptor heap was modified
 * commands.Barrier({StageFlags::Compute, StageFlags::FragmentShader, HazardFlags::Descriptors});
 * 
 * // Render target to shader read
 * commands.Barrier({StageFlags::ColorAttachmentOut, StageFlags::FragmentShader});
 * @endcode
 */
export struct BarrierCommand {
	StageFlags srcStages = StageFlags::None;   ///< Source pipeline stages (what to wait for)
	StageFlags dstStages = StageFlags::None;   ///< Destination stages (what will execute after)
	HazardFlags hazards  = HazardFlags::None;  ///< Special cache hazard handling
	
	constexpr BarrierCommand() noexcept = default;
	
	constexpr BarrierCommand(StageFlags src, StageFlags dst, 
	                         HazardFlags hazardFlags = HazardFlags::None) noexcept
		: srcStages(src)
		, dstStages(dst)
		, hazards(hazardFlags)
	{}
	
	/**
	 * @brief Create standard compute UAV barrier
	 */
	[[nodiscard]] static constexpr BarrierCommand ComputeUAV() noexcept {
		return {StageFlags::Compute, StageFlags::Compute};
	}
	
	/**
	 * @brief Create compute-to-compute barrier with descriptor hazard
	 */
	[[nodiscard]] static constexpr BarrierCommand ComputeWithDescriptors() noexcept {
		return {StageFlags::Compute, StageFlags::Compute, HazardFlags::Descriptors};
	}
	
	/**
	 * @brief Create render-target-to-shader-read barrier
	 */
	[[nodiscard]] static constexpr BarrierCommand RenderTargetToShaderRead() noexcept {
		return {StageFlags::ColorAttachmentOut | StageFlags::DepthStencilOut,
		        StageFlags::FragmentShader};
	}
	
	/**
	 * @brief Create indirect arguments barrier
	 */
	[[nodiscard]] static constexpr BarrierCommand IndirectArguments() noexcept {
		return {StageFlags::Compute, StageFlags::VertexInput, HazardFlags::DrawArguments};
	}
	
	/**
	 * @brief Create full pipeline barrier (use sparingly)
	 */
	[[nodiscard]] static constexpr BarrierCommand Full() noexcept {
		return {StageFlags::AllCommands, StageFlags::AllCommands};
	}
};

/**
 * @brief Memory barrier scope for global synchronization
 * 
 * Used for explicit memory domain transitions when needed.
 * Most operations don't require this on modern GPUs.
 */
export enum class MemoryDomain : std::uint8_t {
	Device,     ///< GPU device memory
	Host,       ///< CPU host memory  
	Queue,      ///< Queue family ownership
};

/**
 * @brief Global memory barrier (rarely needed on modern GPUs)
 */
export struct MemoryBarrierCommand {
	StageFlags srcStages = StageFlags::None;
	StageFlags dstStages = StageFlags::None;
	MemoryDomain srcDomain = MemoryDomain::Device;
	MemoryDomain dstDomain = MemoryDomain::Device;
	
	constexpr MemoryBarrierCommand() noexcept = default;
	
	/**
	 * @brief Create host-to-device transfer completion barrier
	 */
	[[nodiscard]] static constexpr MemoryBarrierCommand HostToDevice() noexcept {
		return {StageFlags::Host, StageFlags::AllCommands, 
		        MemoryDomain::Host, MemoryDomain::Device};
	}
	
	/**
	 * @brief Create device-to-host readback barrier
	 */
	[[nodiscard]] static constexpr MemoryBarrierCommand DeviceToHost() noexcept {
		return {StageFlags::AllCommands, StageFlags::Host,
		        MemoryDomain::Device, MemoryDomain::Host};
	}
};

