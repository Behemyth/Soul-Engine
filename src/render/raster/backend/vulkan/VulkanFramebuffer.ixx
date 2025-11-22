
export module synodic.soul.raster.backend.vulkan:framebuffer;
import std;
import vulkan_hpp;
import std;
import :render_pass;

export class VulkanFrameBuffer{

public:

	VulkanFrameBuffer(const vk::Device& device,
		std::span<vk::ImageView>,
		VulkanRenderPass&,
		vk::Extent2D&);
	~VulkanFrameBuffer();

	VulkanFrameBuffer(const VulkanFrameBuffer&) = delete;
	VulkanFrameBuffer(VulkanFrameBuffer&&) noexcept = default;

	VulkanFrameBuffer& operator=(const VulkanFrameBuffer&) = delete;
	VulkanFrameBuffer& operator=(VulkanFrameBuffer&&) noexcept = default;

	const vk::Framebuffer& Handle() const;

private:

	vk::Device device_;
	vk::Framebuffer frameBuffer_;


};
