#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

class VulkanCommandPool;
class VulkanDevice;

class VulkanCommandBuffer final {

public:

	VulkanCommandBuffer(const vk::CommandPool&,
		const vk::Device&,
		vk::CommandBufferUsageFlagBits,
		vk::CommandBufferLevel);
	~VulkanCommandBuffer();

	VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
	VulkanCommandBuffer(VulkanCommandBuffer&&) noexcept = default;

	VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
	VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) noexcept = default;

	void Begin();
	void End();

	const vk::CommandBuffer& Handle() const;


private:

	vk::CommandPool commandPool_;

	vk::Device device_;


	vk::CommandBufferUsageFlagBits usage_;
	vk::CommandBuffer commandBuffer_;

};
