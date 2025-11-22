

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
	// TODO: Should be conditionally included when the class is only debug mode.
	// TODO: Fix callback signature to work with C++20 modules (needs C Vulkan types)

	// static VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
	// 	VkDebugUtilsMessageTypeFlagsEXT,
	// 	const VkDebugUtilsMessengerCallbackDataEXT*,
	// 	void*);
	 
	vk::DebugUtilsMessengerEXT debugMessenger_;

};
