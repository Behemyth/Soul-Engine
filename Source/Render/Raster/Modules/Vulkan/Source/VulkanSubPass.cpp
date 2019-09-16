#include "VulkanSubPass.h"

VulkanSubPass::VulkanSubPass(const nonstd::span<vk::AttachmentReference2KHR> outputAttachmentReferences)
{
	
	vk::SubpassDescription2KHR& subPass = description_;
	subPass.flags = vk::SubpassDescriptionFlags();
	subPass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subPass.viewMask = 0;
	subPass.inputAttachmentCount = 0;
	subPass.pInputAttachments = nullptr;
	subPass.colorAttachmentCount = outputAttachmentReferences.size();
	subPass.pColorAttachments = outputAttachmentReferences.data();
	subPass.pResolveAttachments = nullptr;
	subPass.pDepthStencilAttachment = nullptr;
	subPass.preserveAttachmentCount = 0;
	subPass.pPreserveAttachments = nullptr;

}

const vk::SubpassDescription2KHR& VulkanSubPass::Description() const
{
	
	return description_;
	
}