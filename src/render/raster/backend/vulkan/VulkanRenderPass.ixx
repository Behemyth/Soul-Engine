module;

#include <vulkan/vulkan.hpp>

export module render.raster.vulkan:render_pass;

import synodic.soul.engine;
import :device;

export class VulkanRenderPass
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
