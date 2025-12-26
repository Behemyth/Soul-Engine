/**
 * @file texture_heap.ixx
 * @brief Bindless texture descriptor heap
 * 
 * Implements a global texture descriptor array following "No Graphics API" patterns.
 * Textures are referenced by 32-bit indices instead of descriptor set bindings,
 * enabling fully bindless rendering.
 * 
 * The heap manages 256-bit descriptors (hardware-specific format) in a single
 * large GPU buffer. Shaders access textures via:
 *   Texture2D tex = textureHeap[textureIndex];
 * 
 * @see https://www.sebastianaaltonen.com/blog/no-graphics-api
 */
export module synodic.soul.raster:texture_heap;

import std;
import :gpu_pointer;
import :types;

/**
 * @brief Strongly-typed 32-bit texture index
 * 
 * References a texture in the global heap. 32-bit indices are sufficient
 * for millions of textures while being half the size of 64-bit handles.
 */
export struct TextureIndex {
	std::uint32_t value = InvalidIndex;
	
	static constexpr std::uint32_t InvalidIndex = std::numeric_limits<std::uint32_t>::max();
	
	constexpr TextureIndex() noexcept = default;
	constexpr explicit TextureIndex(std::uint32_t idx) noexcept : value(idx) {}
	
	[[nodiscard]] constexpr bool IsValid() const noexcept {
		return value != InvalidIndex;
	}
	
	[[nodiscard]] constexpr explicit operator bool() const noexcept {
		return IsValid();
	}
	
	[[nodiscard]] constexpr explicit operator std::uint32_t() const noexcept {
		return value;
	}
	
	constexpr bool operator==(const TextureIndex&) const noexcept = default;
	constexpr auto operator<=>(const TextureIndex&) const noexcept = default;
};

/**
 * @brief Invalid texture index sentinel
 */
export constexpr TextureIndex InvalidTextureIndex{};

/**
 * @brief Sampler configuration for texture sampling
 */
export enum class SamplerFilter : std::uint8_t {
	Nearest,
	Linear,
	Cubic,  // Requires extension on some platforms
};

export enum class SamplerAddressMode : std::uint8_t {
	Repeat,
	MirroredRepeat,
	ClampToEdge,
	ClampToBorder,
};

export enum class SamplerBorderColor : std::uint8_t {
	TransparentBlack,
	OpaqueBlack,
	OpaqueWhite,
};

/**
 * @brief Sampler index for bindless sampler access
 */
export struct SamplerIndex {
	std::uint32_t value = InvalidIndex;
	
	static constexpr std::uint32_t InvalidIndex = std::numeric_limits<std::uint32_t>::max();
	
	constexpr SamplerIndex() noexcept = default;
	constexpr explicit SamplerIndex(std::uint32_t idx) noexcept : value(idx) {}
	
	[[nodiscard]] constexpr bool IsValid() const noexcept {
		return value != InvalidIndex;
	}
	
	[[nodiscard]] constexpr explicit operator bool() const noexcept {
		return IsValid();
	}
	
	constexpr bool operator==(const SamplerIndex&) const noexcept = default;
};

export constexpr SamplerIndex InvalidSamplerIndex{};

/**
 * @brief Sampler creation parameters
 */
export struct SamplerDesc {
	SamplerFilter minFilter = SamplerFilter::Linear;
	SamplerFilter magFilter = SamplerFilter::Linear;
	SamplerFilter mipFilter = SamplerFilter::Linear;
	SamplerAddressMode addressU = SamplerAddressMode::Repeat;
	SamplerAddressMode addressV = SamplerAddressMode::Repeat;
	SamplerAddressMode addressW = SamplerAddressMode::Repeat;
	SamplerBorderColor borderColor = SamplerBorderColor::OpaqueBlack;
	float mipLodBias = 0.0f;
	float minLod = 0.0f;
	float maxLod = 1000.0f;  // VK_LOD_CLAMP_NONE equivalent
	bool anisotropyEnable = true;
	float maxAnisotropy = 16.0f;
	bool compareEnable = false;
	// CompareOp handled separately for depth sampling
	
	/**
	 * @brief Default trilinear filtering sampler
	 */
	[[nodiscard]] static constexpr SamplerDesc Trilinear() noexcept {
		return {};
	}
	
	/**
	 * @brief Point/nearest sampling (no filtering)
	 */
	[[nodiscard]] static constexpr SamplerDesc Point() noexcept {
		SamplerDesc desc;
		desc.minFilter = SamplerFilter::Nearest;
		desc.magFilter = SamplerFilter::Nearest;
		desc.mipFilter = SamplerFilter::Nearest;
		desc.anisotropyEnable = false;
		return desc;
	}
	
	/**
	 * @brief Bilinear filtering (no mip interpolation)
	 */
	[[nodiscard]] static constexpr SamplerDesc Bilinear() noexcept {
		SamplerDesc desc;
		desc.mipFilter = SamplerFilter::Nearest;
		return desc;
	}
	
	/**
	 * @brief Clamped sampler (no texture wrapping)
	 */
	[[nodiscard]] static constexpr SamplerDesc Clamped() noexcept {
		SamplerDesc desc;
		desc.addressU = SamplerAddressMode::ClampToEdge;
		desc.addressV = SamplerAddressMode::ClampToEdge;
		desc.addressW = SamplerAddressMode::ClampToEdge;
		return desc;
	}
};

/**
 * @brief Texture view type for descriptor creation
 */
export enum class TextureViewType : std::uint8_t {
	Texture1D,
	Texture1DArray,
	Texture2D,
	Texture2DArray,
	Texture2DMS,       // Multisample
	Texture2DMSArray,
	TextureCube,
	TextureCubeArray,
	Texture3D,
};

/**
 * @brief Component swizzle for texture views
 */
export enum class ComponentSwizzle : std::uint8_t {
	Identity,
	Zero,
	One,
	R,
	G,
	B,
	A,
};

/**
 * @brief Texture view parameters for descriptor creation
 */
export struct TextureViewDesc {
	TextureViewType viewType = TextureViewType::Texture2D;
	Format format = Format::RGBA8_UNORM;
	
	// Subresource range
	std::uint32_t baseMipLevel = 0;
	std::uint32_t mipLevelCount = 1;
	std::uint32_t baseArrayLayer = 0;
	std::uint32_t arrayLayerCount = 1;
	
	// Component mapping (usually identity)
	ComponentSwizzle swizzleR = ComponentSwizzle::Identity;
	ComponentSwizzle swizzleG = ComponentSwizzle::Identity;
	ComponentSwizzle swizzleB = ComponentSwizzle::Identity;
	ComponentSwizzle swizzleA = ComponentSwizzle::Identity;
	
	/**
	 * @brief Simple 2D texture view with all mips
	 */
	[[nodiscard]] static constexpr TextureViewDesc Texture2D(Format fmt, std::uint32_t mipCount = 1) noexcept {
		TextureViewDesc desc;
		desc.format = fmt;
		desc.mipLevelCount = mipCount;
		return desc;
	}
	
	/**
	 * @brief Cubemap view
	 */
	[[nodiscard]] static constexpr TextureViewDesc Cubemap(Format fmt, std::uint32_t mipCount = 1) noexcept {
		TextureViewDesc desc;
		desc.viewType = TextureViewType::TextureCube;
		desc.format = fmt;
		desc.mipLevelCount = mipCount;
		desc.arrayLayerCount = 6;
		return desc;
	}
	
	/**
	 * @brief 2D texture array view
	 */
	[[nodiscard]] static constexpr TextureViewDesc Array2D(Format fmt, std::uint32_t layers, 
	                                                        std::uint32_t mipCount = 1) noexcept {
		TextureViewDesc desc;
		desc.viewType = TextureViewType::Texture2DArray;
		desc.format = fmt;
		desc.mipLevelCount = mipCount;
		desc.arrayLayerCount = layers;
		return desc;
	}
};

/**
 * @brief 256-bit hardware texture descriptor
 * 
 * This is a placeholder for the actual hardware-specific descriptor format.
 * Real implementation will use VK_EXT_descriptor_buffer to create these.
 * 
 * The descriptor contains all information needed for texture sampling:
 * - Base address
 * - Dimensions
 * - Format
 * - Mip levels
 * - etc.
 * 
 * Size is 256 bits (32 bytes) which matches common GPU descriptor sizes.
 */
export struct alignas(32) TextureDescriptor {
	std::array<std::uint64_t, 4> data{};  // 256 bits / 32 bytes
	
	constexpr TextureDescriptor() noexcept = default;
	
	[[nodiscard]] constexpr bool IsValid() const noexcept {
		// Non-zero first qword indicates valid descriptor
		return data[0] != 0;
	}
};

static_assert(sizeof(TextureDescriptor) == 32, "TextureDescriptor must be 256 bits");
static_assert(alignof(TextureDescriptor) == 32, "TextureDescriptor must be 32-byte aligned");

/**
 * @brief Error codes for texture heap operations
 */
export enum class TextureHeapError {
	Success = 0,
	HeapFull,
	InvalidHandle,
	InvalidFormat,
	AllocationFailed,
	NotInitialized,
};

/**
 * @brief Result type for texture heap operations
 */
export template<typename T>
using TextureHeapResult = std::expected<T, TextureHeapError>;

/**
 * @brief Abstract texture heap interface
 * 
 * Backends implement this to provide platform-specific bindless texture
 * descriptor management. The heap maintains a single large array of
 * descriptors that shaders can index into directly.
 * 
 * Usage:
 * @code
 * // Create texture and add to heap
 * auto texture = CreateTexture(...);
 * TextureIndex idx = heap.Allocate(texture, viewDesc);
 * 
 * // Pass index to shader via root arguments
 * shaderData.albedoTexture = idx.value;
 * 
 * // In shader (HLSL/GLSL):
 * // Texture2D textures[] : register(t0, space0);
 * // color = textures[albedoTexture].Sample(sampler, uv);
 * @endcode
 * 
 * Thread Safety: Allocate/Free must be externally synchronized.
 * Update may be called from any thread if targeting different indices.
 */
export class TextureHeap {
public:
	TextureHeap() = default;
	virtual ~TextureHeap() = default;
	
	TextureHeap(const TextureHeap&) = delete;
	TextureHeap& operator=(const TextureHeap&) = delete;
	TextureHeap(TextureHeap&&) noexcept = default;
	TextureHeap& operator=(TextureHeap&&) noexcept = default;
	
	/**
	 * @brief Get maximum number of textures the heap can hold
	 */
	[[nodiscard]] virtual std::uint32_t Capacity() const noexcept = 0;
	
	/**
	 * @brief Get number of currently allocated texture slots
	 */
	[[nodiscard]] virtual std::uint32_t Size() const noexcept = 0;
	
	/**
	 * @brief Check if heap has available slots
	 */
	[[nodiscard]] virtual bool HasSpace() const noexcept {
		return Size() < Capacity();
	}
	
	/**
	 * @brief Allocate a texture slot and write descriptor
	 * 
	 * @param nativeHandle Backend-specific texture handle (e.g., VkImageView)
	 * @param viewDesc View parameters for descriptor creation
	 * @return TextureIndex on success, error on failure
	 */
	[[nodiscard]] virtual TextureHeapResult<TextureIndex> Allocate(
		std::uint64_t nativeHandle, 
		const TextureViewDesc& viewDesc) = 0;
	
	/**
	 * @brief Free a texture slot
	 * 
	 * The slot becomes available for reuse. The texture resource
	 * itself is not destroyed - that's the caller's responsibility.
	 * 
	 * @param index Index to free
	 */
	virtual void Free(TextureIndex index) = 0;
	
	/**
	 * @brief Update an existing texture descriptor
	 * 
	 * Useful for streaming textures or updating mip levels.
	 * 
	 * @param index Index to update
	 * @param nativeHandle New texture handle
	 * @param viewDesc New view parameters
	 */
	virtual void Update(TextureIndex index, 
	                    std::uint64_t nativeHandle,
	                    const TextureViewDesc& viewDesc) = 0;
	
	/**
	 * @brief Get GPU address of descriptor heap base
	 * 
	 * This address is set as the active texture heap in command buffers.
	 * Shaders compute texture addresses as: heapBase + index * 32
	 */
	[[nodiscard]] virtual GPUDeviceAddress GPUAddress() const noexcept = 0;
	
	/**
	 * @brief Batch allocate multiple texture slots
	 * 
	 * More efficient than individual allocations for loading texture arrays.
	 * 
	 * @param handles Array of native texture handles
	 * @param viewDescs Array of view descriptions (same size as handles)
	 * @return Vector of allocated indices, or error
	 */
	[[nodiscard]] virtual TextureHeapResult<std::vector<TextureIndex>> AllocateBatch(
		std::span<const std::uint64_t> handles,
		std::span<const TextureViewDesc> viewDescs) {
		
		if (handles.size() != viewDescs.size()) {
			return std::unexpected(TextureHeapError::InvalidHandle);
		}
		
		std::vector<TextureIndex> indices;
		indices.reserve(handles.size());
		
		for (std::size_t i = 0; i < handles.size(); ++i) {
			auto result = Allocate(handles[i], viewDescs[i]);
			if (!result) {
				// Rollback on failure
				for (auto& idx : indices) {
					Free(idx);
				}
				return std::unexpected(result.error());
			}
			indices.push_back(*result);
		}
		
		return indices;
	}
};

/**
 * @brief Sampler heap for bindless sampler access
 * 
 * Similar to TextureHeap but for sampler objects. On some platforms,
 * samplers are combined with textures; on others, they're separate.
 */
export class SamplerHeap {
public:
	SamplerHeap() = default;
	virtual ~SamplerHeap() = default;
	
	SamplerHeap(const SamplerHeap&) = delete;
	SamplerHeap& operator=(const SamplerHeap&) = delete;
	SamplerHeap(SamplerHeap&&) noexcept = default;
	SamplerHeap& operator=(SamplerHeap&&) noexcept = default;
	
	[[nodiscard]] virtual std::uint32_t Capacity() const noexcept = 0;
	[[nodiscard]] virtual std::uint32_t Size() const noexcept = 0;
	
	/**
	 * @brief Create or find existing sampler matching description
	 * 
	 * Samplers are typically deduplicated since there are limited
	 * unique sampling configurations in practice.
	 */
	[[nodiscard]] virtual TextureHeapResult<SamplerIndex> GetOrCreate(const SamplerDesc& desc) = 0;
	
	/**
	 * @brief Get GPU address of sampler heap
	 */
	[[nodiscard]] virtual GPUDeviceAddress GPUAddress() const noexcept = 0;
};

