/**
 * @file vulkan_texture_heap.ixx
 * @brief Vulkan implementation of bindless texture descriptor heap
 * 
 * Uses VK_EXT_descriptor_buffer to create a GPU-resident descriptor heap
 * that shaders can index into directly via 32-bit indices.
 */
export module synodic.soul.raster.backend.vulkan:texture_heap;

import std;
import vulkan;

import synodic.soul.raster;
import :error;
import :allocator;

/**
 * @brief Vulkan implementation of bindless texture heap
 * 
 * Creates a descriptor buffer containing sampled image descriptors.
 * Textures are referenced by 32-bit indices in shader data structs.
 */
export class VulkanTextureHeap final : public TextureHeap {
public:
	/**
	 * @brief Create texture heap with specified capacity
	 * 
	 * @param device Vulkan logical device
	 * @param physicalDevice Physical device for descriptor size queries
	 * @param allocator VMA allocator for buffer allocation
	 * @param capacity Maximum number of textures
	 */
	VulkanTextureHeap(
		vk::Device device,
		vk::PhysicalDevice physicalDevice,
		VulkanAllocator& allocator,
		std::uint32_t capacity = DefaultCapacity);
	
	~VulkanTextureHeap() override;
	
	VulkanTextureHeap(const VulkanTextureHeap&) = delete;
	VulkanTextureHeap& operator=(const VulkanTextureHeap&) = delete;
	
	VulkanTextureHeap(VulkanTextureHeap&& other) noexcept;
	VulkanTextureHeap& operator=(VulkanTextureHeap&& other) noexcept;
	
	// TextureHeap interface
	[[nodiscard]] std::uint32_t Capacity() const noexcept override { return capacity_; }
	[[nodiscard]] std::uint32_t Size() const noexcept override { return allocatedCount_; }
	
	[[nodiscard]] TextureHeapResult<TextureIndex> Allocate(
		std::uint64_t nativeHandle,
		const TextureViewDesc& viewDesc) override;
	
	void Free(TextureIndex index) override;
	
	void Update(TextureIndex index,
	            std::uint64_t nativeHandle,
	            const TextureViewDesc& viewDesc) override;
	
	[[nodiscard]] GPUDeviceAddress GPUAddress() const noexcept override {
		return gpuAddress_;
	}
	
	/**
	 * @brief Get descriptor buffer handle for pipeline binding
	 */
	[[nodiscard]] vk::Buffer DescriptorBuffer() const noexcept {
		return descriptorBuffer_;
	}
	
	/**
	 * @brief Get descriptor size for this device
	 */
	[[nodiscard]] std::size_t DescriptorSize() const noexcept {
		return descriptorSize_;
	}
	
	static constexpr std::uint32_t DefaultCapacity = 16384;  // 16K textures

private:
	void WriteDescriptor(std::uint32_t index, vk::ImageView imageView, const TextureViewDesc& viewDesc);
	[[nodiscard]] std::uint32_t AllocateIndex();
	void FreeIndex(std::uint32_t index);
	
	vk::Device device_ = nullptr;
	VulkanAllocator* allocator_ = nullptr;
	
	vk::Buffer descriptorBuffer_ = nullptr;
	VmaAllocationHandle allocation_ = nullptr;
	void* mappedPtr_ = nullptr;
	GPUDeviceAddress gpuAddress_ = InvalidGPUAddress;
	
	std::uint32_t capacity_ = 0;
	std::uint32_t allocatedCount_ = 0;
	std::size_t descriptorSize_ = 0;
	
	// Free list for slot reuse
	std::vector<std::uint32_t> freeIndices_;
	std::uint32_t nextFreeIndex_ = 0;
};

// Implementation

VulkanTextureHeap::VulkanTextureHeap(
	vk::Device device,
	vk::PhysicalDevice physicalDevice,
	VulkanAllocator& allocator,
	std::uint32_t capacity)
	: device_(device)
	, allocator_(&allocator)
	, capacity_(capacity)
{
	// Query descriptor sizes from device
	vk::PhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProps;
	vk::PhysicalDeviceProperties2 props2;
	props2.pNext = &descriptorBufferProps;
	physicalDevice.getProperties2(&props2);
	
	descriptorSize_ = descriptorBufferProps.sampledImageDescriptorSize;
	
	// Create descriptor buffer
	std::size_t bufferSize = capacity_ * descriptorSize_;
	
	// Set up allocation info
	VulkanAllocationCreateInfo allocInfo;
	allocInfo.usage = MemoryUsage::Auto;
	allocInfo.mapped = true;  // Persistently mapped for descriptor writes
	
	// Buffer usage flags for descriptor buffer
	vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT |
	                             vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT |
	                             vk::BufferUsageFlagBits::eShaderDeviceAddress;
	
	auto result = allocator_->CreateBuffer(bufferSize, usage, allocInfo);
	
	if (!result) {
		throw std::runtime_error("Failed to create texture heap descriptor buffer");
	}
	
	descriptorBuffer_ = result->buffer;
	allocation_ = result->allocation;
	mappedPtr_ = result->mappedData;
	
	// Get GPU address
	vk::BufferDeviceAddressInfo addressInfo;
	addressInfo.buffer = descriptorBuffer_;
	gpuAddress_ = device_.getBufferAddress(addressInfo);
	
	// Reserve free list capacity
	freeIndices_.reserve(capacity_ / 4);  // Assume 25% turnover
}

VulkanTextureHeap::~VulkanTextureHeap() {
	if (descriptorBuffer_ && allocator_) {
		allocator_->DestroyBuffer(descriptorBuffer_, allocation_);
	}
}

VulkanTextureHeap::VulkanTextureHeap(VulkanTextureHeap&& other) noexcept
	: device_(other.device_)
	, allocator_(other.allocator_)
	, descriptorBuffer_(other.descriptorBuffer_)
	, allocation_(other.allocation_)
	, mappedPtr_(other.mappedPtr_)
	, gpuAddress_(other.gpuAddress_)
	, capacity_(other.capacity_)
	, allocatedCount_(other.allocatedCount_)
	, descriptorSize_(other.descriptorSize_)
	, freeIndices_(std::move(other.freeIndices_))
	, nextFreeIndex_(other.nextFreeIndex_)
{
	other.device_ = nullptr;
	other.allocator_ = nullptr;
	other.descriptorBuffer_ = nullptr;
	other.allocation_ = nullptr;
	other.mappedPtr_ = nullptr;
	other.gpuAddress_ = InvalidGPUAddress;
}

VulkanTextureHeap& VulkanTextureHeap::operator=(VulkanTextureHeap&& other) noexcept {
	if (this != &other) {
		if (descriptorBuffer_ && allocator_) {
			allocator_->DestroyBuffer(descriptorBuffer_, allocation_);
		}
		
		device_ = other.device_;
		allocator_ = other.allocator_;
		descriptorBuffer_ = other.descriptorBuffer_;
		allocation_ = other.allocation_;
		mappedPtr_ = other.mappedPtr_;
		gpuAddress_ = other.gpuAddress_;
		capacity_ = other.capacity_;
		allocatedCount_ = other.allocatedCount_;
		descriptorSize_ = other.descriptorSize_;
		freeIndices_ = std::move(other.freeIndices_);
		nextFreeIndex_ = other.nextFreeIndex_;
		
		other.device_ = nullptr;
		other.allocator_ = nullptr;
		other.descriptorBuffer_ = nullptr;
		other.allocation_ = nullptr;
		other.mappedPtr_ = nullptr;
		other.gpuAddress_ = InvalidGPUAddress;
	}
	return *this;
}

TextureHeapResult<TextureIndex> VulkanTextureHeap::Allocate(
	std::uint64_t nativeHandle,
	const TextureViewDesc& viewDesc)
{
	if (allocatedCount_ >= capacity_) {
		return std::unexpected(TextureHeapError::HeapFull);
	}
	
	std::uint32_t index = AllocateIndex();
	
	// Native handle is VkImageView - use vk::ImageView constructor
	vk::ImageView imageView{reinterpret_cast<VkImageView>(nativeHandle)};
	
	WriteDescriptor(index, imageView, viewDesc);
	
	return TextureIndex{index};
}

void VulkanTextureHeap::Free(TextureIndex index) {
	if (!index.IsValid() || index.value >= capacity_) {
		return;
	}
	
	FreeIndex(index.value);
}

void VulkanTextureHeap::Update(
	TextureIndex index,
	std::uint64_t nativeHandle,
	const TextureViewDesc& viewDesc)
{
	if (!index.IsValid() || index.value >= capacity_) {
		return;
	}
	
	// Native handle is VkImageView - use vk::ImageView constructor
	vk::ImageView imageView{reinterpret_cast<VkImageView>(nativeHandle)};
	
	WriteDescriptor(index.value, imageView, viewDesc);
}

void VulkanTextureHeap::WriteDescriptor(
	std::uint32_t index,
	vk::ImageView imageView,
	const TextureViewDesc& viewDesc)
{
	// Calculate offset into descriptor buffer
	std::byte* dstPtr = static_cast<std::byte*>(mappedPtr_) + (index * descriptorSize_);
	
	// Create descriptor info
	vk::DescriptorImageInfo imageInfo;
	imageInfo.imageView = imageView;
	imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	imageInfo.sampler = nullptr;  // Separate sampler heap
	
	// Get descriptor data
	vk::DescriptorGetInfoEXT getInfo;
	getInfo.type = vk::DescriptorType::eSampledImage;
	getInfo.data.pSampledImage = &imageInfo;
	
	// Write descriptor directly to buffer
	device_.getDescriptorEXT(&getInfo, descriptorSize_, dstPtr);
}

std::uint32_t VulkanTextureHeap::AllocateIndex() {
	std::uint32_t index;
	
	if (!freeIndices_.empty()) {
		index = freeIndices_.back();
		freeIndices_.pop_back();
	} else {
		index = nextFreeIndex_++;
	}
	
	++allocatedCount_;
	return index;
}

void VulkanTextureHeap::FreeIndex(std::uint32_t index) {
	freeIndices_.push_back(index);
	--allocatedCount_;
}

