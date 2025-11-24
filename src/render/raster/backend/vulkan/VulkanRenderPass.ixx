
export module synodic.soul.raster.backend.vulkan:render_pass;
import std;
import vulkan_hpp;
import :device;
import synodic.soul.scheduler;

export template<SchedulerBackend SchedulerType>
class VulkanRenderPass
{

public:

	VulkanRenderPass(const VulkanDevice<SchedulerType>&,
		std::span<vk::AttachmentDescription2> subPassAttachments,
		std::span<vk::SubpassDescription2> subPassDescriptions,
		std::span<vk::SubpassDependency2> subPassDependencies);

	~VulkanRenderPass() {
		device_.destroyRenderPass(renderPass_);
	}

	VulkanRenderPass(const VulkanRenderPass&) = delete;
	VulkanRenderPass(VulkanRenderPass&&) noexcept = default;

	VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;
	VulkanRenderPass& operator=(VulkanRenderPass&&) noexcept = default;

	const vk::RenderPass& Handle() const {
		return renderPass_;
	}

private:

	vk::Device device_;
	vk::RenderPass renderPass_;

};

// Template implementation
template<SchedulerBackend SchedulerType>
VulkanRenderPass<SchedulerType>::VulkanRenderPass(const VulkanDevice<SchedulerType>& device,
	std::span<vk::AttachmentDescription2> subPassAttachments,
	std::span<vk::SubpassDescription2> subPassDescriptions,
	std::span<vk::SubpassDependency2> subPassDependencies):
	device_(device.Logical())
{

	vk::RenderPassCreateInfo2 renderPassInfo;
	renderPassInfo.flags = vk::RenderPassCreateFlags();
	renderPassInfo.attachmentCount = static_cast<std::uint32_t>(subPassAttachments.size());
	renderPassInfo.pAttachments = subPassAttachments.data();
	renderPassInfo.subpassCount = static_cast<std::uint32_t>(subPassDescriptions.size());
	renderPassInfo.pSubpasses = subPassDescriptions.data();
	renderPassInfo.dependencyCount = static_cast<std::uint32_t>(subPassDependencies.size());
	renderPassInfo.pDependencies = subPassDependencies.data();
	renderPassInfo.correlatedViewMaskCount = 0;
	renderPassInfo.pCorrelatedViewMasks = nullptr;

	renderPass_ = device_.createRenderPass2(renderPassInfo);

}
