module synodic.soul.raster.backend.vulkan;

VulkanSubPass::VulkanSubPass(const std::span<vk::AttachmentReference2KHR> outputAttachmentReferences) :
	colorAttachments_(outputAttachmentReferences.begin(), outputAttachmentReferences.end())
{
	vk::SubpassDescription2KHR& subPass = description_;
	subPass.sType = vk::StructureType::eSubpassDescription2;
	subPass.pNext = nullptr;
	subPass.flags = vk::SubpassDescriptionFlags();
	subPass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subPass.viewMask = 0;
	subPass.inputAttachmentCount = 0;
	subPass.pInputAttachments = nullptr;
	subPass.colorAttachmentCount = static_cast<std::uint32_t>(colorAttachments_.size());
	subPass.pColorAttachments = colorAttachments_.data();
	subPass.pResolveAttachments = nullptr;
	subPass.pDepthStencilAttachment = nullptr;
	subPass.preserveAttachmentCount = 0;
	subPass.pPreserveAttachments = nullptr;
}

VulkanSubPass::VulkanSubPass(const std::span<vk::AttachmentReference2KHR> outputAttachmentReferences,
	const vk::AttachmentReference2KHR& depthAttachment) :
	colorAttachments_(outputAttachmentReferences.begin(), outputAttachmentReferences.end()),
	depthAttachment_(depthAttachment)
{
	vk::SubpassDescription2KHR& subPass = description_;
	subPass.sType = vk::StructureType::eSubpassDescription2;
	subPass.pNext = nullptr;
	subPass.flags = vk::SubpassDescriptionFlags();
	subPass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subPass.viewMask = 0;
	subPass.inputAttachmentCount = 0;
	subPass.pInputAttachments = nullptr;
	subPass.colorAttachmentCount = static_cast<std::uint32_t>(colorAttachments_.size());
	subPass.pColorAttachments = colorAttachments_.data();
	subPass.pResolveAttachments = nullptr;
	subPass.pDepthStencilAttachment = &depthAttachment_.value();
	subPass.preserveAttachmentCount = 0;
	subPass.pPreserveAttachments = nullptr;
}

VulkanSubPass::VulkanSubPass(const VulkanSubPass& other) :
	colorAttachments_(other.colorAttachments_),
	depthAttachment_(other.depthAttachment_),
	description_(other.description_)
{
	UpdateDescriptionPointers();
}

VulkanSubPass::VulkanSubPass(VulkanSubPass&& other) noexcept :
	colorAttachments_(std::move(other.colorAttachments_)),
	depthAttachment_(std::move(other.depthAttachment_)),
	description_(other.description_)
{
	UpdateDescriptionPointers();
}

VulkanSubPass& VulkanSubPass::operator=(const VulkanSubPass& other)
{
	if (this != &other) {
		colorAttachments_ = other.colorAttachments_;
		depthAttachment_ = other.depthAttachment_;
		description_ = other.description_;
		UpdateDescriptionPointers();
	}
	return *this;
}

VulkanSubPass& VulkanSubPass::operator=(VulkanSubPass&& other) noexcept
{
	if (this != &other) {
		colorAttachments_ = std::move(other.colorAttachments_);
		depthAttachment_ = std::move(other.depthAttachment_);
		description_ = other.description_;
		UpdateDescriptionPointers();
	}
	return *this;
}

void VulkanSubPass::UpdateDescriptionPointers()
{
	description_.colorAttachmentCount = static_cast<std::uint32_t>(colorAttachments_.size());
	description_.pColorAttachments = colorAttachments_.empty() ? nullptr : colorAttachments_.data();
	description_.pDepthStencilAttachment = depthAttachment_.has_value() ? &depthAttachment_.value() : nullptr;
}

const vk::SubpassDescription2KHR& VulkanSubPass::Description() const
{
	return description_;
}
