module;

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS	 1
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

module synodic.soul.raster.backend.vulkan;

VulkanAllocator::VulkanAllocator(
	vk::Instance instance,
	vk::PhysicalDevice physicalDevice,
	vk::Device device,
	std::uint32_t vulkanApiVersion) :
	device_(device)
{
	VmaAllocatorCreateInfo allocatorInfo {};
	allocatorInfo.vulkanApiVersion = vulkanApiVersion;
	allocatorInfo.instance		   = static_cast<VkInstance>(instance);
	allocatorInfo.physicalDevice   = static_cast<VkPhysicalDevice>(physicalDevice);
	allocatorInfo.device		   = static_cast<VkDevice>(device);
	
	// Enable buffer device address for bindless rendering (BDA/GPU pointers)
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

	// With VMA_STATIC_VULKAN_FUNCTIONS=1, VMA calls Vulkan functions directly
	// No need to provide pVulkanFunctions
	allocatorInfo.pVulkanFunctions = nullptr;

	VmaAllocator vmaAllocator;
	VkResult result = vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create VMA allocator: " + std::to_string(result));
	}
	allocator_ = reinterpret_cast<VmaAllocatorHandle>(vmaAllocator);
}

VulkanAllocator::~VulkanAllocator()
{
	if (allocator_)
	{
		vmaDestroyAllocator(reinterpret_cast<VmaAllocator>(allocator_));
		allocator_ = nullptr;
	}
}

VulkanAllocator::VulkanAllocator(VulkanAllocator&& other) noexcept :
	allocator_(other.allocator_),
	device_(other.device_)
{
	other.allocator_ = nullptr;
}

VulkanAllocator& VulkanAllocator::operator=(VulkanAllocator&& other) noexcept
{
	if (this != &other)
	{
		if (allocator_)
		{
			vmaDestroyAllocator(reinterpret_cast<VmaAllocator>(allocator_));
		}
		allocator_		 = other.allocator_;
		device_			 = other.device_;
		other.allocator_ = nullptr;
	}
	return *this;
}

VulkanResult<VulkanAllocation> VulkanAllocator::CreateBuffer(
	vk::DeviceSize size,
	vk::BufferUsageFlags usage,
	const VulkanAllocationCreateInfo& allocInfo)
{
	VkBufferCreateInfo bufferInfo {};
	bufferInfo.sType	   = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size		   = size;
	bufferInfo.usage	   = static_cast<VkBufferUsageFlags>(usage);
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo vmaAllocInfo {};
	vmaAllocInfo.usage			= static_cast<VmaMemoryUsage>(static_cast<int>(allocInfo.usage));
	vmaAllocInfo.requiredFlags	= static_cast<VkMemoryPropertyFlags>(allocInfo.requiredFlags);
	vmaAllocInfo.preferredFlags = static_cast<VkMemoryPropertyFlags>(allocInfo.preferredFlags);

	if (allocInfo.mapped)
	{
		vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	}

	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo allocationInfo;

	VkResult result = vmaCreateBuffer(
		reinterpret_cast<VmaAllocator>(allocator_),
		&bufferInfo,
		&vmaAllocInfo,
		&buffer,
		&allocation,
		&allocationInfo);

	if (result != VK_SUCCESS)
	{
		return std::unexpected(FromVkResult(static_cast<vk::Result>(result)));
	}

	VulkanAllocation vulkanAlloc;
	vulkanAlloc.allocation = reinterpret_cast<VmaAllocationHandle>(allocation);
	vulkanAlloc.mappedData = allocationInfo.pMappedData;
	vulkanAlloc.buffer	   = static_cast<vk::Buffer>(buffer);
	return vulkanAlloc;
}

void VulkanAllocator::DestroyBuffer(vk::Buffer buffer, VmaAllocationHandle allocation)
{
	vmaDestroyBuffer(
		reinterpret_cast<VmaAllocator>(allocator_),
		static_cast<VkBuffer>(buffer),
		reinterpret_cast<VmaAllocation>(allocation));
}

VulkanResult<VulkanAllocation>
	VulkanAllocator::CreateImage(const vk::ImageCreateInfo& imageInfo, const VulkanAllocationCreateInfo& allocInfo)
{
	VmaAllocationCreateInfo vmaAllocInfo {};
	vmaAllocInfo.usage			= static_cast<VmaMemoryUsage>(static_cast<int>(allocInfo.usage));
	vmaAllocInfo.requiredFlags	= static_cast<VkMemoryPropertyFlags>(allocInfo.requiredFlags);
	vmaAllocInfo.preferredFlags = static_cast<VkMemoryPropertyFlags>(allocInfo.preferredFlags);

	VkImage image;
	VmaAllocation allocation;
	VmaAllocationInfo allocationInfo;

	VkResult result = vmaCreateImage(
		reinterpret_cast<VmaAllocator>(allocator_),
		reinterpret_cast<const VkImageCreateInfo*>(&imageInfo),
		&vmaAllocInfo,
		&image,
		&allocation,
		&allocationInfo);

	if (result != VK_SUCCESS)
	{
		return std::unexpected(FromVkResult(static_cast<vk::Result>(result)));
	}

	VulkanAllocation vulkanAlloc;
	vulkanAlloc.allocation = reinterpret_cast<VmaAllocationHandle>(allocation);
	vulkanAlloc.mappedData = allocationInfo.pMappedData;
	vulkanAlloc.image	   = static_cast<vk::Image>(image);
	return vulkanAlloc;
}

void VulkanAllocator::DestroyImage(vk::Image image, VmaAllocationHandle allocation)
{
	vmaDestroyImage(
		reinterpret_cast<VmaAllocator>(allocator_),
		static_cast<VkImage>(image),
		reinterpret_cast<VmaAllocation>(allocation));
}

VulkanResult<void*> VulkanAllocator::MapMemory(VmaAllocationHandle allocation)
{
	void* data;
	VkResult result =
		vmaMapMemory(reinterpret_cast<VmaAllocator>(allocator_), reinterpret_cast<VmaAllocation>(allocation), &data);
	if (result != VK_SUCCESS)
	{
		return std::unexpected(FromVkResult(static_cast<vk::Result>(result)));
	}

	return data;
}

void VulkanAllocator::UnmapMemory(VmaAllocationHandle allocation)
{
	vmaUnmapMemory(reinterpret_cast<VmaAllocator>(allocator_), reinterpret_cast<VmaAllocation>(allocation));
}

VulkanVoidResult
	VulkanAllocator::FlushAllocation(VmaAllocationHandle allocation, vk::DeviceSize offset, vk::DeviceSize size)
{
	VkResult result = vmaFlushAllocation(
		reinterpret_cast<VmaAllocator>(allocator_),
		reinterpret_cast<VmaAllocation>(allocation),
		static_cast<VkDeviceSize>(offset),
		static_cast<VkDeviceSize>(size));
	if (result != VK_SUCCESS)
	{
		return std::unexpected(FromVkResult(static_cast<vk::Result>(result)));
	}

	return {};
}

VulkanVoidResult
	VulkanAllocator::InvalidateAllocation(VmaAllocationHandle allocation, vk::DeviceSize offset, vk::DeviceSize size)
{
	VkResult result = vmaInvalidateAllocation(
		reinterpret_cast<VmaAllocator>(allocator_),
		reinterpret_cast<VmaAllocation>(allocation),
		static_cast<VkDeviceSize>(offset),
		static_cast<VkDeviceSize>(size));
	if (result != VK_SUCCESS)
	{
		return std::unexpected(FromVkResult(static_cast<vk::Result>(result)));
	}

	return {};
}
