#include "VulkanFrame.h"

VulkanFrameBuffer& VulkanFrame::Framebuffer()
{
	
	return framebuffer_.value();
	
}

VulkanSemaphore& VulkanFrame::RenderSemaphore()
{
	
	return renderSemaphore_.value();
	
}