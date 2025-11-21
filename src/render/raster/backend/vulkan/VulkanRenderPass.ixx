
export module render.raster.vulkan:render_pass;
import std;
import vulkan_hpp;
import :device;

export class VulkanRenderPass
{

public:

	VulkanRenderPass(const VulkanDevice&,
		std::span<vk::AttachmentDescription2> subPassAttachments,
		std::span<vk::SubpassDescription2> subPassDescriptions,
		std::span<vk::SubpassDependency2> subPassDependencies);
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
