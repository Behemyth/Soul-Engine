export module synodic.soul.render.graph:pass;

import std;
import synodic.soul.core;
import synodic.soul.raster;

// Forward declarations for resource handles
export using ResourceHandle = std::uint32_t;
export constexpr ResourceHandle InvalidResourceHandle = std::numeric_limits<ResourceHandle>::max();

// Re-export ResourceUsage from raster types for convenience
// (ResourceUsage is defined in synodic.soul.raster:types as it's API-agnostic)

// Transient image descriptor - for resources that exist only within the frame graph
export struct TransientImageDesc {
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	Format format = Format::RGBA;
	std::uint32_t mipLevels = 1;
	std::uint32_t arrayLayers = 1;
	bool depthStencil = false;

	// Compute memory requirements for aliasing
	[[nodiscard]] constexpr std::size_t MemorySize() const noexcept {
		// Simplified: actual size depends on format bytes per pixel
		std::size_t bpp = (format == Format::RGBA) ? 4 : 4;
		return static_cast<std::size_t>(width) * height * bpp * mipLevels * arrayLayers;
	}

	constexpr bool operator==(const TransientImageDesc&) const = default;
};

// Transient buffer descriptor
export struct TransientBufferDesc {
	std::size_t size = 0;
	BufferType type = BufferType::Storage;

	constexpr bool operator==(const TransientBufferDesc&) const = default;
};

// Resource reference - tracks how a pass uses a resource
export struct ResourceRef {
	ResourceHandle handle = InvalidResourceHandle;
	ResourceUsage usage = ResourceUsage::None;

	constexpr bool IsValid() const noexcept { return handle != InvalidResourceHandle; }
	constexpr bool IsRead() const noexcept {
		return HasFlag(usage, ResourceUsage::VertexShaderRead) ||
		       HasFlag(usage, ResourceUsage::FragmentShaderRead) ||
		       HasFlag(usage, ResourceUsage::ComputeShaderRead) ||
		       HasFlag(usage, ResourceUsage::TransferSource) ||
		       HasFlag(usage, ResourceUsage::DepthStencilRead) ||
		       HasFlag(usage, ResourceUsage::ColorAttachmentRead);
	}
	constexpr bool IsWrite() const noexcept {
		return HasFlag(usage, ResourceUsage::FragmentShaderWrite) ||
		       HasFlag(usage, ResourceUsage::ComputeShaderWrite) ||
		       HasFlag(usage, ResourceUsage::TransferDest) ||
		       HasFlag(usage, ResourceUsage::DepthStencilWrite) ||
		       HasFlag(usage, ResourceUsage::ColorAttachmentWrite);
	}
};

// Pass types - determines execution queue and merging rules
export enum class PassType {
	Raster,   // Graphics queue, can merge into subpasses
	Compute,  // Compute queue, standalone
	Transfer, // Transfer queue, standalone
};

// Render pass descriptor
export struct RenderPassDesc {
	std::string name;
	PassType type = PassType::Raster;
	ShaderSet shaders = {};

	// Attachment descriptions
	std::vector<ResourceRef> colorOutputs;
	std::optional<ResourceRef> depthStencilOutput;
	std::vector<ResourceRef> inputs;

	// Clear values
	std::array<float, 4> clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
	float clearDepth = 1.0f;
	std::uint32_t clearStencil = 0;
	bool shouldClear = true;

	// Load/store behavior
	bool loadPreviousContent = false;
	bool storeResult = true;

	// Index in original order (for stable sorting)
	std::uint32_t originalIndex = 0;

	// Merged pass tracking
	bool isMerged = false;
	std::uint32_t mergedIntoPass = 0;
};

// Compute pass descriptor
export struct ComputePassDesc {
	std::string name;
	ShaderSet shaders = {};

	std::vector<ResourceRef> inputs;
	std::vector<ResourceRef> outputs;

	uvec2 dispatchSize = {1, 1};

	std::uint32_t originalIndex = 0;
};

// Transfer pass descriptor
export struct TransferPassDesc {
	std::string name;

	ResourceRef source;
	ResourceRef destination;

	std::uint32_t originalIndex = 0;
};

// Unified pass handle - type-erased reference to any pass
export struct PassHandle {
	PassType type = PassType::Raster;
	std::uint32_t index = 0;

	constexpr bool operator==(const PassHandle&) const = default;
	constexpr auto operator<=>(const PassHandle&) const = default;
};

// Resource lifetime tracking for aliasing
export struct ResourceLifetime {
	ResourceHandle handle = InvalidResourceHandle;
	std::uint32_t firstUse = std::numeric_limits<std::uint32_t>::max();  // First pass that uses this
	std::uint32_t lastUse = 0;   // Last pass that uses this
	std::size_t memorySize = 0;  // Size in bytes for aliasing

	// Can this resource alias with another (non-overlapping lifetimes)?
	[[nodiscard]] constexpr bool CanAliasWith(const ResourceLifetime& other) const noexcept {
		return lastUse < other.firstUse || other.lastUse < firstUse;
	}
};

// Merged pass group - tracks which passes were merged into Vulkan subpasses
export struct MergedPassGroup {
	std::vector<std::uint32_t> passIndices;  // Original pass indices
	std::vector<ResourceHandle> colorAttachments;
	std::optional<ResourceHandle> depthAttachment;

	// All unique resources accessed by this group
	std::vector<ResourceRef> allInputs;
	std::vector<ResourceRef> allOutputs;
};

