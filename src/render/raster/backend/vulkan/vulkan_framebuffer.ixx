
export module synodic.soul.raster.backend.vulkan:framebuffer;
import std;
import vulkan;
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
		if (frameBuffer_) {
			device_.destroyFramebuffer(frameBuffer_);
		}
	}

	VulkanFrameBuffer(const VulkanFrameBuffer&) = delete;

	VulkanFrameBuffer(VulkanFrameBuffer&& other) noexcept :
		device_(other.device_),
		frameBuffer_(other.frameBuffer_)
	{
		other.frameBuffer_ = nullptr;
		other.device_ = nullptr;
	}

	VulkanFrameBuffer& operator=(const VulkanFrameBuffer&) = delete;

	VulkanFrameBuffer& operator=(VulkanFrameBuffer&& other) noexcept {
		if (this != &other) {
			if (frameBuffer_) {
				device_.destroyFramebuffer(frameBuffer_);
			}
			device_ = other.device_;
			frameBuffer_ = other.frameBuffer_;
			other.frameBuffer_ = nullptr;
			other.device_ = nullptr;
		}
		return *this;
	}

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
