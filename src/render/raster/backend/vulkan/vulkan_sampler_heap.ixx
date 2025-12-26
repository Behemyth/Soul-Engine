/**
 * @file vulkan_sampler_heap.ixx
 * @brief Vulkan implementation of bindless sampler descriptor heap
 * 
 * Uses VK_EXT_descriptor_buffer to create a GPU-resident sampler heap.
 * Samplers are deduplicated based on SamplerDesc hash to minimize allocations.
 */
export module synodic.soul.raster.backend.vulkan:sampler_heap;

import std;
import vulkan;

import synodic.soul.raster;
import :error;
import :allocator;

/**
 * @brief Vulkan implementation of bindless sampler heap
 * 
 * Creates a descriptor buffer containing sampler descriptors.
 * Samplers are deduplicated by their configuration hash.
 */
export class VulkanSamplerHeap final : public SamplerHeap {
public:
	/**
	 * @brief Create sampler heap with specified capacity
	 */
	VulkanSamplerHeap(
		vk::Device device,
		vk::PhysicalDevice physicalDevice,
		VulkanAllocator& allocator,
		std::uint32_t capacity = DefaultCapacity);
	
	~VulkanSamplerHeap() override;
	
	VulkanSamplerHeap(const VulkanSamplerHeap&) = delete;
	VulkanSamplerHeap& operator=(const VulkanSamplerHeap&) = delete;
	
	VulkanSamplerHeap(VulkanSamplerHeap&& other) noexcept;
	VulkanSamplerHeap& operator=(VulkanSamplerHeap&& other) noexcept;
	
	// SamplerHeap interface
	[[nodiscard]] std::uint32_t Capacity() const noexcept override { return capacity_; }
	[[nodiscard]] std::uint32_t Size() const noexcept override { 
		return static_cast<std::uint32_t>(samplerCache_.size()); 
	}
	
	[[nodiscard]] TextureHeapResult<SamplerIndex> GetOrCreate(const SamplerDesc& desc) override;
	
	[[nodiscard]] GPUDeviceAddress GPUAddress() const noexcept override {
		return gpuAddress_;
	}
	
	/**
	 * @brief Get descriptor buffer handle
	 */
	[[nodiscard]] vk::Buffer DescriptorBuffer() const noexcept {
		return descriptorBuffer_;
	}
	
	static constexpr std::uint32_t DefaultCapacity = 256;  // Usually < 100 unique samplers

private:
	[[nodiscard]] std::size_t HashSamplerDesc(const SamplerDesc& desc) const noexcept;
	[[nodiscard]] vk::Sampler CreateVulkanSampler(const SamplerDesc& desc);
	void WriteDescriptor(std::uint32_t index, vk::Sampler sampler);
	
	vk::Device device_ = nullptr;
	VulkanAllocator* allocator_ = nullptr;
	
	vk::Buffer descriptorBuffer_ = nullptr;
	VmaAllocationHandle allocation_ = nullptr;
	void* mappedPtr_ = nullptr;
	GPUDeviceAddress gpuAddress_ = InvalidGPUAddress;
	
	std::uint32_t capacity_ = 0;
	std::size_t descriptorSize_ = 0;
	std::uint32_t nextIndex_ = 0;
	
	// Cache: hash -> (index, sampler handle)
	struct SamplerEntry {
		SamplerIndex index;
		vk::Sampler sampler;
	};
	std::unordered_map<std::size_t, SamplerEntry> samplerCache_;
	
	// Track all created samplers for cleanup
	std::vector<vk::Sampler> samplers_;
};

// Helper to convert our enums to Vulkan
namespace {
	constexpr vk::Filter ToVkFilter(SamplerFilter filter) noexcept {
		switch (filter) {
			case SamplerFilter::Nearest: return vk::Filter::eNearest;
			case SamplerFilter::Linear:  return vk::Filter::eLinear;
			case SamplerFilter::Cubic:   return vk::Filter::eCubicIMG;
			default: return vk::Filter::eLinear;
		}
	}
	
	constexpr vk::SamplerMipmapMode ToVkMipmapMode(SamplerFilter filter) noexcept {
		switch (filter) {
			case SamplerFilter::Nearest: return vk::SamplerMipmapMode::eNearest;
			case SamplerFilter::Linear:  return vk::SamplerMipmapMode::eLinear;
			case SamplerFilter::Cubic:   return vk::SamplerMipmapMode::eLinear;
			default: return vk::SamplerMipmapMode::eLinear;
		}
	}
	
	constexpr vk::SamplerAddressMode ToVkAddressMode(SamplerAddressMode mode) noexcept {
		switch (mode) {
			case SamplerAddressMode::Repeat:         return vk::SamplerAddressMode::eRepeat;
			case SamplerAddressMode::MirroredRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
			case SamplerAddressMode::ClampToEdge:    return vk::SamplerAddressMode::eClampToEdge;
			case SamplerAddressMode::ClampToBorder:  return vk::SamplerAddressMode::eClampToBorder;
			default: return vk::SamplerAddressMode::eRepeat;
		}
	}
	
	constexpr vk::BorderColor ToVkBorderColor(SamplerBorderColor color) noexcept {
		switch (color) {
			case SamplerBorderColor::TransparentBlack: return vk::BorderColor::eFloatTransparentBlack;
			case SamplerBorderColor::OpaqueBlack:      return vk::BorderColor::eFloatOpaqueBlack;
			case SamplerBorderColor::OpaqueWhite:      return vk::BorderColor::eFloatOpaqueWhite;
			default: return vk::BorderColor::eFloatOpaqueBlack;
		}
	}
}

// Implementation

VulkanSamplerHeap::VulkanSamplerHeap(
	vk::Device device,
	vk::PhysicalDevice physicalDevice,
	VulkanAllocator& allocator,
	std::uint32_t capacity)
	: device_(device)
	, allocator_(&allocator)
	, capacity_(capacity)
{
	// Query descriptor sizes
	vk::PhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProps;
	vk::PhysicalDeviceProperties2 props2;
	props2.pNext = &descriptorBufferProps;
	physicalDevice.getProperties2(&props2);
	
	descriptorSize_ = descriptorBufferProps.samplerDescriptorSize;
	
	// Create descriptor buffer
	std::size_t bufferSize = capacity_ * descriptorSize_;
	
	VulkanAllocationCreateInfo allocInfo;
	allocInfo.usage = MemoryUsage::Auto;
	allocInfo.mapped = true;
	
	vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT |
	                             vk::BufferUsageFlagBits::eShaderDeviceAddress;
	
	auto result = allocator_->CreateBuffer(bufferSize, usage, allocInfo);
	
	if (!result) {
		throw std::runtime_error("Failed to create sampler heap descriptor buffer");
	}
	
	descriptorBuffer_ = result->buffer;
	allocation_ = result->allocation;
	mappedPtr_ = result->mappedData;
	
	// Get GPU address
	vk::BufferDeviceAddressInfo addressInfo;
	addressInfo.buffer = descriptorBuffer_;
	gpuAddress_ = device_.getBufferAddress(addressInfo);
	
	samplers_.reserve(capacity_);
}

VulkanSamplerHeap::~VulkanSamplerHeap() {
	// Destroy all samplers
	for (auto sampler : samplers_) {
		if (sampler) {
			device_.destroySampler(sampler);
		}
	}
	
	if (descriptorBuffer_ && allocator_) {
		allocator_->DestroyBuffer(descriptorBuffer_, allocation_);
	}
}

VulkanSamplerHeap::VulkanSamplerHeap(VulkanSamplerHeap&& other) noexcept
	: device_(other.device_)
	, allocator_(other.allocator_)
	, descriptorBuffer_(other.descriptorBuffer_)
	, allocation_(other.allocation_)
	, mappedPtr_(other.mappedPtr_)
	, gpuAddress_(other.gpuAddress_)
	, capacity_(other.capacity_)
	, descriptorSize_(other.descriptorSize_)
	, nextIndex_(other.nextIndex_)
	, samplerCache_(std::move(other.samplerCache_))
	, samplers_(std::move(other.samplers_))
{
	other.device_ = nullptr;
	other.allocator_ = nullptr;
	other.descriptorBuffer_ = nullptr;
	other.allocation_ = nullptr;
	other.mappedPtr_ = nullptr;
	other.gpuAddress_ = InvalidGPUAddress;
}

VulkanSamplerHeap& VulkanSamplerHeap::operator=(VulkanSamplerHeap&& other) noexcept {
	if (this != &other) {
		// Cleanup existing
		for (auto sampler : samplers_) {
			if (sampler) {
				device_.destroySampler(sampler);
			}
		}
		if (descriptorBuffer_ && allocator_) {
			allocator_->DestroyBuffer(descriptorBuffer_, allocation_);
		}
		
		// Move
		device_ = other.device_;
		allocator_ = other.allocator_;
		descriptorBuffer_ = other.descriptorBuffer_;
		allocation_ = other.allocation_;
		mappedPtr_ = other.mappedPtr_;
		gpuAddress_ = other.gpuAddress_;
		capacity_ = other.capacity_;
		descriptorSize_ = other.descriptorSize_;
		nextIndex_ = other.nextIndex_;
		samplerCache_ = std::move(other.samplerCache_);
		samplers_ = std::move(other.samplers_);
		
		other.device_ = nullptr;
		other.allocator_ = nullptr;
		other.descriptorBuffer_ = nullptr;
		other.allocation_ = nullptr;
		other.mappedPtr_ = nullptr;
		other.gpuAddress_ = InvalidGPUAddress;
	}
	return *this;
}

TextureHeapResult<SamplerIndex> VulkanSamplerHeap::GetOrCreate(const SamplerDesc& desc) {
	std::size_t hash = HashSamplerDesc(desc);
	
	// Check cache
	auto it = samplerCache_.find(hash);
	if (it != samplerCache_.end()) {
		return it->second.index;
	}
	
	// Create new sampler
	if (nextIndex_ >= capacity_) {
		return std::unexpected(TextureHeapError::HeapFull);
	}
	
	vk::Sampler sampler = CreateVulkanSampler(desc);
	std::uint32_t index = nextIndex_++;
	
	WriteDescriptor(index, sampler);
	
	SamplerIndex samplerIndex{index};
	samplerCache_[hash] = {samplerIndex, sampler};
	samplers_.push_back(sampler);
	
	return samplerIndex;
}

std::size_t VulkanSamplerHeap::HashSamplerDesc(const SamplerDesc& desc) const noexcept {
	// FNV-1a hash of sampler parameters
	std::size_t hash = 14695981039346656037ULL;
	
	auto hashByte = [&hash](std::uint8_t b) {
		hash ^= b;
		hash *= 1099511628211ULL;
	};
	
	hashByte(static_cast<std::uint8_t>(desc.minFilter));
	hashByte(static_cast<std::uint8_t>(desc.magFilter));
	hashByte(static_cast<std::uint8_t>(desc.mipFilter));
	hashByte(static_cast<std::uint8_t>(desc.addressU));
	hashByte(static_cast<std::uint8_t>(desc.addressV));
	hashByte(static_cast<std::uint8_t>(desc.addressW));
	hashByte(static_cast<std::uint8_t>(desc.borderColor));
	hashByte(desc.anisotropyEnable ? 1 : 0);
	hashByte(desc.compareEnable ? 1 : 0);
	
	// Hash floats
	auto hashFloat = [&hash](float f) {
		std::uint32_t bits;
		std::memcpy(&bits, &f, sizeof(bits));
		hash ^= bits;
		hash *= 1099511628211ULL;
	};
	
	hashFloat(desc.mipLodBias);
	hashFloat(desc.minLod);
	hashFloat(desc.maxLod);
	hashFloat(desc.maxAnisotropy);
	
	return hash;
}

vk::Sampler VulkanSamplerHeap::CreateVulkanSampler(const SamplerDesc& desc) {
	vk::SamplerCreateInfo createInfo;
	createInfo.magFilter = ToVkFilter(desc.magFilter);
	createInfo.minFilter = ToVkFilter(desc.minFilter);
	createInfo.mipmapMode = ToVkMipmapMode(desc.mipFilter);
	createInfo.addressModeU = ToVkAddressMode(desc.addressU);
	createInfo.addressModeV = ToVkAddressMode(desc.addressV);
	createInfo.addressModeW = ToVkAddressMode(desc.addressW);
	createInfo.mipLodBias = desc.mipLodBias;
	createInfo.anisotropyEnable = desc.anisotropyEnable ? vk::True : vk::False;
	createInfo.maxAnisotropy = desc.maxAnisotropy;
	createInfo.compareEnable = desc.compareEnable ? vk::True : vk::False;
	createInfo.compareOp = vk::CompareOp::eLessOrEqual;  // Default for shadow sampling
	createInfo.minLod = desc.minLod;
	createInfo.maxLod = desc.maxLod;
	createInfo.borderColor = ToVkBorderColor(desc.borderColor);
	createInfo.unnormalizedCoordinates = vk::False;
	
	return device_.createSampler(createInfo);
}

void VulkanSamplerHeap::WriteDescriptor(std::uint32_t index, vk::Sampler sampler) {
	std::byte* dstPtr = static_cast<std::byte*>(mappedPtr_) + (index * descriptorSize_);
	
	vk::DescriptorGetInfoEXT getInfo;
	getInfo.type = vk::DescriptorType::eSampler;
	getInfo.data.pSampler = &sampler;
	
	device_.getDescriptorEXT(&getInfo, descriptorSize_, dstPtr);
}

