#pragma once

#include <vulkan/vulkan.hpp>

#include "Render/Raster/Modules/Vulkan/Device/VulkanDevice.h"
#include "Types.h"


class VulkanDevice;

template <typename T>
class VulkanBuffer {

public:

	VulkanBuffer(vk::DeviceSize, const vk::BufferUsageFlags&, const vk::MemoryPropertyFlags&, const std::shared_ptr<VulkanDevice>&);
	virtual ~VulkanBuffer();

	VulkanBuffer(const VulkanBuffer&) = delete;
	VulkanBuffer(VulkanBuffer&& o) noexcept = delete;

	VulkanBuffer& operator=(const VulkanBuffer&) = delete;
	VulkanBuffer& operator=(VulkanBuffer&& other) noexcept = delete;


    const vk::Buffer& GetBuffer() const;

    T* Map(); //TODO: Return std::span C++20
	void UnMap() const;


private:

    uint FindMemoryType(uint, const vk::MemoryPropertyFlags&) const;


    vk::DeviceSize size_;
	vk::Buffer buffer_;
	vk::DeviceMemory deviceMemory_;

    std::shared_ptr<VulkanDevice> device_;


};

template <typename T>
VulkanBuffer<T>::VulkanBuffer(vk::DeviceSize size, const vk::BufferUsageFlags& bufferUsage, const vk::MemoryPropertyFlags& properties, const std::shared_ptr<VulkanDevice>& device) : 																																											 
    size_(size),
    buffer_(nullptr),																																													 
    deviceMemory_(nullptr),																																													 
    device_(device) 
{

	const vk::Device& logicalDevice = device_->GetLogical();

	std::array<uint32, 1> queueFamily = { device_->GetGraphicsIndex() };

	vk::BufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.size = size_ * sizeof(T);
	bufferCreateInfo.usage = bufferUsage;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	bufferCreateInfo.queueFamilyIndexCount = queueFamily.size();
	bufferCreateInfo.pQueueFamilyIndices = queueFamily.data();

	buffer_ = logicalDevice.createBuffer(bufferCreateInfo);

	const vk::MemoryRequirements memoryRequirements = logicalDevice.getBufferMemoryRequirements(buffer_);

	vk::MemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties);

	deviceMemory_ = logicalDevice.allocateMemory(memoryAllocateInfo);

	logicalDevice.bindBufferMemory(buffer_, deviceMemory_, 0);

}

template <typename T>
VulkanBuffer<T>::~VulkanBuffer() {

	const vk::Device& logicalDevice = device_->GetLogical();

	logicalDevice.destroyBuffer(buffer_);
	logicalDevice.freeMemory(deviceMemory_);

}

template <typename T>
const vk::Buffer& VulkanBuffer<T>::GetBuffer() const {

	return buffer_;

}

template <typename T>
T* VulkanBuffer<T>::Map() {

	const vk::Device& logicalDevice = device_->GetLogical();

	return static_cast<T*>(logicalDevice.mapMemory(deviceMemory_, 0, size_ * sizeof(T)));

}

template <typename T>
void VulkanBuffer<T>::UnMap() const {

	const vk::Device& logicalDevice = device_->GetLogical();

	logicalDevice.unmapMemory(deviceMemory_);

}

template <typename T>
uint VulkanBuffer<T>::FindMemoryType(const uint typeFilter, const vk::MemoryPropertyFlags& properties) const {

	const vk::PhysicalDevice& physicalDevice = device_->GetPhysical();

	vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

	for(uint i = 0; i < memoryProperties.memoryTypeCount; i++) {

		const bool correctType = typeFilter & 1 << i;
		const bool requiredProperties = (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;

		if(correctType && requiredProperties) {
			return i;
		}
	}

	throw std::runtime_error("Memory type not found");

}