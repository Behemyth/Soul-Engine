/**
 * @file vulkan_gpu_allocator.ixx
 * @brief Vulkan GPU pointer allocator for bindless root arguments
 * 
 * Allocates GPU memory with buffer device addresses for use as
 * shader root arguments following "No Graphics API" patterns.
 */
export module synodic.soul.raster.backend.vulkan:gpu_allocator;

import std;
import vulkan;

import synodic.soul.raster;
import :error;
import :allocator;

/**
 * @brief Vulkan GPU pointer allocator
 * 
 * Provides allocation of GPU memory with paired CPU/GPU addresses
 * for use as shader root arguments.
 * 
 * Memory is allocated with:
 * - VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT for GPU pointer access
 * - HOST_VISIBLE | HOST_COHERENT for direct CPU writes (ReBAR/UMA)
 * 
 * Thread Safety: Allocate/Free must be externally synchronized.
 */
export class VulkanGPUAllocator {
public:
	/**
	 * @brief Create GPU allocator
	 * 
	 * @param device Vulkan logical device
	 * @param physicalDevice Physical device for memory property queries
	 * @param allocator VMA allocator
	 */
	VulkanGPUAllocator(
		vk::Device device,
		vk::PhysicalDevice physicalDevice,
		VulkanAllocator& allocator);
	
	~VulkanGPUAllocator();
	
	VulkanGPUAllocator(const VulkanGPUAllocator&) = delete;
	VulkanGPUAllocator& operator=(const VulkanGPUAllocator&) = delete;
	
	VulkanGPUAllocator(VulkanGPUAllocator&& other) noexcept;
	VulkanGPUAllocator& operator=(VulkanGPUAllocator&& other) noexcept;
	
	/**
	 * @brief Allocate GPU memory with CPU mapping
	 * 
	 * @tparam T Type to allocate (must be GPUTransferable)
	 * @param count Number of elements
	 * @return GPUPointer on success, error on failure
	 */
	template<GPUTransferable T>
	[[nodiscard]] VulkanResult<GPUPointer<T>> Allocate(std::size_t count = 1);
	
	/**
	 * @brief Allocate GPU span (array with count)
	 */
	template<GPUTransferable T>
	[[nodiscard]] VulkanResult<GPUSpan<T>> AllocateSpan(std::size_t count);
	
	/**
	 * @brief Free GPU memory
	 * 
	 * The GPUPointer/GPUSpan is invalidated after this call.
	 */
	template<GPUTransferable T>
	void Free(GPUPointer<T>& ptr);
	
	template<GPUTransferable T>
	void Free(GPUSpan<T>& span);
	
	/**
	 * @brief Check if ReBAR (large host-visible GPU memory) is available
	 */
	[[nodiscard]] bool HasReBar() const noexcept { return hasReBar_; }
	
	/**
	 * @brief Get total GPU-accessible memory size
	 */
	[[nodiscard]] std::size_t TotalMemory() const noexcept { return totalHostVisibleGPUMemory_; }

private:
	struct AllocationRecord {
		vk::Buffer buffer = nullptr;
		VmaAllocationHandle allocation = nullptr;
		void* mappedPtr = nullptr;
		GPUDeviceAddress gpuAddress = InvalidGPUAddress;
		std::size_t size = 0;
	};
	
	[[nodiscard]] VulkanResult<AllocationRecord> AllocateInternal(std::size_t sizeBytes);
	void FreeInternal(AllocationRecord& record);
	
	vk::Device device_ = nullptr;
	vk::PhysicalDevice physicalDevice_ = nullptr;
	VulkanAllocator* allocator_ = nullptr;
	
	bool hasReBar_ = false;
	std::size_t totalHostVisibleGPUMemory_ = 0;
	
	// Track allocations by CPU address for deallocation
	std::unordered_map<void*, AllocationRecord> allocations_;
};

// Template implementations

template<GPUTransferable T>
VulkanResult<GPUPointer<T>> VulkanGPUAllocator::Allocate(std::size_t count) {
	std::size_t sizeBytes = sizeof(T) * count;
	
	auto result = AllocateInternal(sizeBytes);
	if (!result) {
		return std::unexpected(result.error());
	}
	
	auto& record = *result;
	allocations_[record.mappedPtr] = record;
	
	return GPUPointer<T>(
		static_cast<T*>(record.mappedPtr),
		record.gpuAddress
	);
}

template<GPUTransferable T>
VulkanResult<GPUSpan<T>> VulkanGPUAllocator::AllocateSpan(std::size_t count) {
	std::size_t sizeBytes = sizeof(T) * count;
	
	auto result = AllocateInternal(sizeBytes);
	if (!result) {
		return std::unexpected(result.error());
	}
	
	auto& record = *result;
	allocations_[record.mappedPtr] = record;
	
	return GPUSpan<T>(
		static_cast<T*>(record.mappedPtr),
		record.gpuAddress,
		count
	);
}

template<GPUTransferable T>
void VulkanGPUAllocator::Free(GPUPointer<T>& ptr) {
	if (!ptr.IsValid()) {
		return;
	}
	
	auto it = allocations_.find(ptr.CPU());
	if (it != allocations_.end()) {
		FreeInternal(it->second);
		allocations_.erase(it);
	}
	
	ptr.Reset();
}

template<GPUTransferable T>
void VulkanGPUAllocator::Free(GPUSpan<T>& span) {
	if (!span.IsValid()) {
		return;
	}
	
	auto it = allocations_.find(span.CPU());
	if (it != allocations_.end()) {
		FreeInternal(it->second);
		allocations_.erase(it);
	}
	
	// Reset span to null state
	span = GPUSpan<T>{};
}

// Non-template implementations

VulkanGPUAllocator::VulkanGPUAllocator(
	vk::Device device,
	vk::PhysicalDevice physicalDevice,
	VulkanAllocator& allocator)
	: device_(device)
	, physicalDevice_(physicalDevice)
	, allocator_(&allocator)
{
	// Query memory properties to detect ReBAR
	vk::PhysicalDeviceMemoryProperties memProps = physicalDevice_.getMemoryProperties();
	
	for (std::uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
		const auto& memType = memProps.memoryTypes[i];
		const auto& heap = memProps.memoryHeaps[memType.heapIndex];
		
		// Look for device-local + host-visible memory (ReBAR)
		bool isDeviceLocal = (memType.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) != vk::MemoryPropertyFlags{};
		bool isHostVisible = (memType.propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible) != vk::MemoryPropertyFlags{};
		
		if (isDeviceLocal && isHostVisible) {
			totalHostVisibleGPUMemory_ = std::max(totalHostVisibleGPUMemory_, 
				static_cast<std::size_t>(heap.size));
		}
	}
	
	// ReBAR typically exposes 256MB+ of host-visible device-local memory
	hasReBar_ = totalHostVisibleGPUMemory_ >= (256 * 1024 * 1024);
}

VulkanGPUAllocator::~VulkanGPUAllocator() {
	// Free any remaining allocations
	for (auto& [ptr, record] : allocations_) {
		FreeInternal(record);
	}
	allocations_.clear();
}

VulkanGPUAllocator::VulkanGPUAllocator(VulkanGPUAllocator&& other) noexcept
	: device_(other.device_)
	, physicalDevice_(other.physicalDevice_)
	, allocator_(other.allocator_)
	, hasReBar_(other.hasReBar_)
	, totalHostVisibleGPUMemory_(other.totalHostVisibleGPUMemory_)
	, allocations_(std::move(other.allocations_))
{
	other.device_ = nullptr;
	other.physicalDevice_ = nullptr;
	other.allocator_ = nullptr;
}

VulkanGPUAllocator& VulkanGPUAllocator::operator=(VulkanGPUAllocator&& other) noexcept {
	if (this != &other) {
		// Free existing allocations
		for (auto& [ptr, record] : allocations_) {
			FreeInternal(record);
		}
		
		device_ = other.device_;
		physicalDevice_ = other.physicalDevice_;
		allocator_ = other.allocator_;
		hasReBar_ = other.hasReBar_;
		totalHostVisibleGPUMemory_ = other.totalHostVisibleGPUMemory_;
		allocations_ = std::move(other.allocations_);
		
		other.device_ = nullptr;
		other.physicalDevice_ = nullptr;
		other.allocator_ = nullptr;
	}
	return *this;
}

VulkanResult<VulkanGPUAllocator::AllocationRecord> VulkanGPUAllocator::AllocateInternal(
	std::size_t sizeBytes)
{
	if (!hasReBar_) {
		return std::unexpected(VulkanError::ReBarNotSupported);
	}
	
	// Set up allocation info for device-local, host-visible memory (ReBAR)
	VulkanAllocationCreateInfo allocInfo;
	allocInfo.usage = MemoryUsage::Auto;
	allocInfo.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal |
	                          vk::MemoryPropertyFlagBits::eHostVisible |
	                          vk::MemoryPropertyFlagBits::eHostCoherent;
	allocInfo.mapped = true;  // Persistently mapped
	
	// Buffer usage flags
	vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eStorageBuffer |
	                             vk::BufferUsageFlagBits::eShaderDeviceAddress |
	                             vk::BufferUsageFlagBits::eTransferSrc |
	                             vk::BufferUsageFlagBits::eTransferDst;
	
	auto result = allocator_->CreateBuffer(sizeBytes, usage, allocInfo);
	
	if (!result) {
		return std::unexpected(VulkanError::AllocationFailed);
	}
	
	AllocationRecord record;
	record.buffer = result->buffer;
	record.allocation = result->allocation;
	record.mappedPtr = result->mappedData;
	record.size = sizeBytes;
	
	// Get GPU device address
	vk::BufferDeviceAddressInfo addressInfo;
	addressInfo.buffer = record.buffer;
	record.gpuAddress = device_.getBufferAddress(addressInfo);
	
	return record;
}

void VulkanGPUAllocator::FreeInternal(AllocationRecord& record) {
	if (record.buffer && allocator_) {
		allocator_->DestroyBuffer(record.buffer, record.allocation);
	}
	record.buffer = nullptr;
	record.allocation = nullptr;
	record.mappedPtr = nullptr;
	record.gpuAddress = InvalidGPUAddress;
}

