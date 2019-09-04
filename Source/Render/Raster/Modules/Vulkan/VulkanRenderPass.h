#pragma once

#include "Types.h"
#include "Device/VulkanDevice.h"

#include <vulkan/vulkan.hpp>

class VulkanRenderPass
{

public:

	VulkanRenderPass(const VulkanDevice&,
		nonstd::span<vk::AttachmentDescription2KHR> subPassAttachments,
		nonstd::span<vk::SubpassDescription2KHR> subPassDescriptions,
		nonstd::span<vk::SubpassDependency2KHR> subPassDependencies);
	~VulkanRenderPass();

	VulkanRenderPass(const VulkanRenderPass&) = delete;
	VulkanRenderPass(VulkanRenderPass&&) noexcept = default;

	VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;
	VulkanRenderPass& operator=(VulkanRenderPass&&) noexcept = default;

	const vk::RenderPass& Handle() const;

private:

	vk::Device device_;
	vk::RenderPass renderPass_;

};
