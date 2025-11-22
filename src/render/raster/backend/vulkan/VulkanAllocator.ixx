export module synodic.soul.raster.backend.vulkan:allocator;

import std;
import vulkan_hpp;
import :error;

// TODO: Custom handles
struct VmaAllocatorOpaque;
struct VmaAllocationOpaque;

export using VmaAllocatorHandle = VmaAllocatorOpaque*;
export using VmaAllocationHandle = VmaAllocationOpaque*;

// Memory usage hints for VMA
export enum class MemoryUsage {
	Unknown = 0,
	GpuOnly = 1,
	CpuOnly = 2,
	CpuToGpu = 3,
	GpuToCpu = 4,
	CpuCopy = 5,
	GpuLazilyAllocated = 6,
	Auto = 7,
	AutoPreferDevice = 8,
	AutoPreferHost = 9
};

// Allocation info for creating buffers/images
export struct VulkanAllocationCreateInfo {
	MemoryUsage usage = MemoryUsage::Auto;
	vk::MemoryPropertyFlags requiredFlags = {};
	vk::MemoryPropertyFlags preferredFlags = {};
	bool mapped = false;  // Keep mapped for persistent mapping
};

// Result of an allocation
export struct VulkanAllocation {
	VmaAllocationHandle allocation = nullptr;
	void* mappedData = nullptr;
	vk::Buffer buffer = nullptr;  // For buffer allocations
	vk::Image image = nullptr;    // For image allocations
};

// VulkanAllocator wraps VMA (Vulkan Memory Allocator) for efficient memory management
export class VulkanAllocator {
public:
	VulkanAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device, std::uint32_t vulkanApiVersion);
	~VulkanAllocator();

	VulkanAllocator(const VulkanAllocator&) = delete;
	VulkanAllocator(VulkanAllocator&& other) noexcept;

	VulkanAllocator& operator=(const VulkanAllocator&) = delete;
	VulkanAllocator& operator=(VulkanAllocator&& other) noexcept;

	// Buffer allocation
	VulkanResult<VulkanAllocation> CreateBuffer(
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		const VulkanAllocationCreateInfo& allocInfo);

	void DestroyBuffer(vk::Buffer buffer, VmaAllocationHandle allocation);

	// Image allocation
	VulkanResult<VulkanAllocation> CreateImage(
		const vk::ImageCreateInfo& imageInfo,
		const VulkanAllocationCreateInfo& allocInfo);

	void DestroyImage(vk::Image image, VmaAllocationHandle allocation);

	// Memory mapping
	VulkanResult<void*> MapMemory(VmaAllocationHandle allocation);
	void UnmapMemory(VmaAllocationHandle allocation);

	// Flush/invalidate for non-coherent memory
	VulkanVoidResult FlushAllocation(VmaAllocationHandle allocation, vk::DeviceSize offset, vk::DeviceSize size);
	VulkanVoidResult InvalidateAllocation(VmaAllocationHandle allocation, vk::DeviceSize offset, vk::DeviceSize size);

	// Get allocator handle (for advanced usage)
	VmaAllocatorHandle Handle() const noexcept { return allocator_; }

private:
	VmaAllocatorHandle allocator_ = nullptr;
	vk::Device device_;
};
