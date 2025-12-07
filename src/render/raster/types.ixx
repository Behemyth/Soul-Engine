export module synodic.soul.raster:types;

import std;
import synodic.library;
import synodic.soul.core;

// Opaque handle for platform/backend-specific surface handles (e.g., VkSurfaceKHR)
// Backends cast to/from their native type using reinterpret_cast
export using NativeSurfaceHandle = std::uint64_t;

export struct ShaderSet
{

	Entity* vertex;

	Entity* tessellationControl;

	Entity* tessellationEvaluation;

	Entity* geometry;

	Entity* fragment;

};

export enum class Format {
	// Color formats
	RGBA,
	RGBA8_SRGB,
	RGBA8_UNORM,
	RGBA16_FLOAT,
	RGBA32_FLOAT,
	RGB10A2_UNORM,
	// Depth formats
	D16_UNORM,
	D32_FLOAT,
	D24_UNORM_S8_UINT,
	D32_FLOAT_S8_UINT,
	// Other
	Unknown
};

// Helper to check if format is depth/stencil
export constexpr bool IsDepthFormat(Format format) {
	return format == Format::D16_UNORM ||
	       format == Format::D32_FLOAT ||
	       format == Format::D24_UNORM_S8_UINT ||
	       format == Format::D32_FLOAT_S8_UINT;
}

export constexpr bool HasStencil(Format format) {
	return format == Format::D24_UNORM_S8_UINT ||
	       format == Format::D32_FLOAT_S8_UINT;
}

// Resource usage flags for synchronization and layout transitions
// These map to equivalent concepts across Vulkan, D3D12, and Metal:
// - Vulkan: VkPipelineStageFlagBits2 + VkAccessFlagBits2
// - D3D12: D3D12_BARRIER_SYNC + D3D12_BARRIER_ACCESS
// - Metal: MTLRenderStages + resource usage flags
export enum class ResourceUsage : std::uint32_t {
	None = 0,
	// Read operations
	VertexShaderRead    = 1 << 0,
	FragmentShaderRead  = 1 << 1,
	ComputeShaderRead   = 1 << 2,
	TransferSource      = 1 << 3,
	DepthStencilRead    = 1 << 4,
	ColorAttachmentRead = 1 << 5,
	// Write operations
	FragmentShaderWrite  = 1 << 6,
	ComputeShaderWrite   = 1 << 7,
	TransferDest         = 1 << 8,
	DepthStencilWrite    = 1 << 9,
	ColorAttachmentWrite = 1 << 10,
	// Present
	Present = 1 << 11,
};

// Enable bitwise operations on ResourceUsage
export constexpr ResourceUsage operator|(ResourceUsage a, ResourceUsage b) {
	return static_cast<ResourceUsage>(
		static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

export constexpr ResourceUsage operator&(ResourceUsage a, ResourceUsage b) {
	return static_cast<ResourceUsage>(
		static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

export constexpr bool HasFlag(ResourceUsage flags, ResourceUsage flag) {
	return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(flag)) != 0;
}
