
export module render.raster.vulkan:subpass;
import std;
import vulkan_hpp;
import :shader;

export class VulkanSubPass
{

public:

	explicit VulkanSubPass(std::span<vk::AttachmentReference2KHR>);
	~VulkanSubPass() = default;

	VulkanSubPass(const VulkanSubPass&) = default;
	VulkanSubPass(VulkanSubPass&&) noexcept = default;

	VulkanSubPass& operator=(const VulkanSubPass&) = default;
	VulkanSubPass& operator=(VulkanSubPass&&) noexcept = default;

	[[nodiscard]] const vk::SubpassDescription2KHR& Description() const;

private:

	vk::SubpassDescription2KHR description_;
	
};
