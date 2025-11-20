module render.raster.vulkan;

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device,
	nonstd::span<vk::AttachmentDescription2KHR> subPassAttachments,
	nonstd::span<vk::SubpassDescription2KHR> subPassDescriptions,
	nonstd::span<vk::SubpassDependency2KHR> subPassDependencies):
	device_(device.Logical())
{

	vk::RenderPassCreateInfo2KHR renderPassInfo;
	renderPassInfo.flags = vk::RenderPassCreateFlags();
	renderPassInfo.attachmentCount = static_cast<std::uint32_t>(subPassAttachments.size());
	renderPassInfo.pAttachments = subPassAttachments.data();
	renderPassInfo.subpassCount = static_cast<std::uint32_t>(subPassDescriptions.size());
	renderPassInfo.pSubpasses = subPassDescriptions.data();
	renderPassInfo.dependencyCount = static_cast<std::uint32_t>(subPassDependencies.size());
	renderPassInfo.pDependencies = subPassDependencies.data();
	renderPassInfo.correlatedViewMaskCount = 0;
	renderPassInfo.pCorrelatedViewMasks = nullptr;

	renderPass_ = device_.createRenderPass2KHR(renderPassInfo, nullptr, device.DispatchLoader());

}

VulkanRenderPass::~VulkanRenderPass() {

	device_.destroyRenderPass(renderPass_);

}

const vk::RenderPass& VulkanRenderPass::Handle() const
{

	return renderPass_;

}
