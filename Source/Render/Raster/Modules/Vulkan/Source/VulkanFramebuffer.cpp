#include "VulkanFramebuffer.h"

#include "VulkanRenderPass.h"

VulkanFrameBuffer::VulkanFrameBuffer(const vk::Device& device,
	nonstd::span<vk::ImageView> attachments,
	VulkanRenderPass& renderPass,
	vk::Extent2D& size):
	device_(device)
{

	vk::FramebufferCreateInfo framebufferInfo;
	framebufferInfo.flags = vk::FramebufferCreateFlags();
	framebufferInfo.renderPass = renderPass.Handle();
	framebufferInfo.attachmentCount = attachments.size();
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = size.width;
	framebufferInfo.height = size.height;
	framebufferInfo.layers = 1;


	frameBuffer_ = device_.createFramebuffer(framebufferInfo);

}

VulkanFrameBuffer::~VulkanFrameBuffer() {

	device_.destroyFramebuffer(frameBuffer_);

}

const vk::Framebuffer& VulkanFrameBuffer::Handle() const
{

	return frameBuffer_;

}
