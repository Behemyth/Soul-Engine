module;

#include <vulkan/vulkan.hpp>

export module render.raster.vulkan:instance;

import synodic.soul.engine;
import :physical_device;

import <string>;
import <vector>;

export class VulkanInstance {

public:

	VulkanInstance(const vk::ApplicationInfo&,
		nonstd::span<std::string>,
		nonstd::span<std::string>);
	~VulkanInstance();

	VulkanInstance(const VulkanInstance&) = default;
	VulkanInstance(VulkanInstance&&) noexcept = default;

	VulkanInstance& operator=(const VulkanInstance&) = default;
	VulkanInstance& operator=(VulkanInstance&&) noexcept = default;

	const vk::Instance& Handle() const;

	std::vector<VulkanPhysicalDevice> EnumeratePhysicalDevices();

private:

	vk::Instance instance_;


	// Dynamic dispatcher for extensions
	vk::DispatchLoaderDynamic dispatcher_;

	// Debug state
	// TODO: Should be conditionally included when the class is only debug mode.

	static VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
		VkDebugUtilsMessageTypeFlagsEXT,
		const VkDebugUtilsMessengerCallbackDataEXT*,
		void*);
	 
	vk::DebugUtilsMessengerEXT debugMessenger_;

};
