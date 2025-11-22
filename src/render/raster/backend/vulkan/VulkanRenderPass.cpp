module synodic.soul.raster.backend.vulkan;

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device,
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

VulkanRenderPass::~VulkanRenderPass() {

	device_.destroyRenderPass(renderPass_);

}

const vk::RenderPass& VulkanRenderPass::Handle() const
{

	return renderPass_;

}
