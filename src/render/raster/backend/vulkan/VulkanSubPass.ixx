
export module synodic.soul.raster.backend.vulkan:subpass;
import std;
import vulkan_hpp;
import :shader;

export class VulkanSubPass
{

public:

	explicit VulkanSubPass(std::span<vk::AttachmentReference2KHR>);
	~VulkanSubPass() = default;

	VulkanSubPass(const VulkanSubPass&);
	VulkanSubPass(VulkanSubPass&&) noexcept;

	VulkanSubPass& operator=(const VulkanSubPass&);
	VulkanSubPass& operator=(VulkanSubPass&&) noexcept;

	[[nodiscard]] const vk::SubpassDescription2KHR& Description() const;

private:

	void UpdateDescriptionPointers();

	std::vector<vk::AttachmentReference2KHR> colorAttachments_;
	vk::SubpassDescription2KHR description_;

};
