

export module synodic.soul.raster.backend.vulkan:instance;
import std;
import vulkan_hpp;
import :physical_device;
import synodic.soul.engine;

export class VulkanInstance {

public:

	VulkanInstance(const vk::ApplicationInfo&,
		std::span<std::string>,
		std::span<std::string>);
	~VulkanInstance();

	VulkanInstance(const VulkanInstance&) = default;
	VulkanInstance(VulkanInstance&&) noexcept = default;

	VulkanInstance& operator=(const VulkanInstance&) = default;
	VulkanInstance& operator=(VulkanInstance&&) noexcept = default;

	const vk::Instance& Handle() const;

	std::vector<VulkanPhysicalDevice> EnumeratePhysicalDevices();

private:

	vk::Instance instance_;

	// Debug state
	vk::DebugUtilsMessengerEXT debugMessenger_;

};
