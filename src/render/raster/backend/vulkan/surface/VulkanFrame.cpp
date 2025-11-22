module synodic.soul.raster.backend.vulkan;

VulkanFrameBuffer& VulkanFrame::Framebuffer()
{

	return framebuffer_.value();

}

VulkanSemaphore& VulkanFrame::RenderSemaphore()
{

	return renderSemaphore_.value();

}
