
export module synodic.soul.raster.backend.vulkan:framebuffer;
import std;
import vulkan_hpp;
import :render_pass;
import synodic.soul.scheduler;

export template<SchedulerBackend SchedulerType>
class VulkanFrameBuffer{

public:

	VulkanFrameBuffer(const vk::Device& device,
		std::span<vk::ImageView>,
		VulkanRenderPass<SchedulerType>&,
		vk::Extent2D&);

	~VulkanFrameBuffer() {
		device_.destroyFramebuffer(frameBuffer_);
	}

	VulkanFrameBuffer(const VulkanFrameBuffer&) = delete;
	VulkanFrameBuffer(VulkanFrameBuffer&&) noexcept = default;

	VulkanFrameBuffer& operator=(const VulkanFrameBuffer&) = delete;
	VulkanFrameBuffer& operator=(VulkanFrameBuffer&&) noexcept = default;

	const vk::Framebuffer& Handle() const {
		return frameBuffer_;
	}

private:

	vk::Device device_;
	vk::Framebuffer frameBuffer_;


};

// Template implementation
template<SchedulerBackend SchedulerType>
VulkanFrameBuffer<SchedulerType>::VulkanFrameBuffer(const vk::Device& device,
	std::span<vk::ImageView> attachments,
	VulkanRenderPass<SchedulerType>& renderPass,
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
