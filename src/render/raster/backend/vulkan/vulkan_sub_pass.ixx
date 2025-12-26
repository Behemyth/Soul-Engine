
export module synodic.soul.raster.backend.vulkan:subpass;
import std;
import vulkan;
import :shader;

export class VulkanSubPass
{

public:

	// Constructor without depth attachment (legacy)
	explicit VulkanSubPass(std::span<vk::AttachmentReference2KHR>);
	
	// Constructor with depth attachment
	VulkanSubPass(std::span<vk::AttachmentReference2KHR>, const vk::AttachmentReference2KHR& depthAttachment);
	
	~VulkanSubPass() = default;

	VulkanSubPass(const VulkanSubPass&);
	VulkanSubPass(VulkanSubPass&&) noexcept;

	VulkanSubPass& operator=(const VulkanSubPass&);
	VulkanSubPass& operator=(VulkanSubPass&&) noexcept;

	[[nodiscard]] const vk::SubpassDescription2KHR& Description() const;

private:

	void UpdateDescriptionPointers();

	std::vector<vk::AttachmentReference2KHR> colorAttachments_;
	std::optional<vk::AttachmentReference2KHR> depthAttachment_;
	vk::SubpassDescription2KHR description_;

};
