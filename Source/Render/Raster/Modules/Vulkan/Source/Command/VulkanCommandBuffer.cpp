#include "VulkanCommandBuffer.h"

#include "VulkanCommandPool.h"

#include "Render/Raster/Modules/Vulkan/Device/VulkanDevice.h"

VulkanCommandBuffer::VulkanCommandBuffer(const vk::CommandPool& commandPool,
	const vk::Device& vulkanDevice,
	vk::CommandBufferUsageFlagBits usage,
	vk::CommandBufferLevel bufferLevel):
	commandPool_(commandPool),
	device_(vulkanDevice), usage_(usage)
{

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.level = bufferLevel;
	allocInfo.commandPool = commandPool_;
	allocInfo.commandBufferCount = 1;

	commandBuffer_ = device_.allocateCommandBuffers(allocInfo).front();

}

VulkanCommandBuffer::~VulkanCommandBuffer()
{

	device_.freeCommandBuffers(commandPool_, commandBuffer_);

}


void VulkanCommandBuffer::Begin()
{

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = usage_;
	beginInfo.pInheritanceInfo = nullptr;

	commandBuffer_.begin(beginInfo);

}

void VulkanCommandBuffer::End()
{

	commandBuffer_.end();

}

const vk::CommandBuffer& VulkanCommandBuffer::Handle() const
{

	return commandBuffer_;

}