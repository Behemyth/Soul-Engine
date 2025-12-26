// Dynamic dispatch initialization for Vulkan-Hpp
// When using VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1, we must provide storage for
// the default dispatch loader. The vulkan module exports the type and symbol name,
// but we need to define the actual storage once in the program.

module synodic.soul.raster.backend.vulkan;

import vulkan;

// Provide storage for the default dynamic dispatcher
// This must be defined exactly once in the entire program
// The vulkan module exports vk::detail::DispatchLoaderDynamic and 
// vk::detail::defaultDispatchLoaderDynamic, we just need to define it
namespace vk::detail
{
	DispatchLoaderDynamic defaultDispatchLoaderDynamic;
}

void InitializeVulkanDispatcher()
{
	// Initialize the dynamic dispatcher with the minimal set of function pointers
	// needed before instance creation (e.g., vkCreateInstance, vkEnumerateInstanceExtensionProperties)
	vk::detail::defaultDispatchLoaderDynamic.init();
}

void InitializeVulkanDispatcherInstance(vk::Instance instance)
{
	// Initialize instance-level function pointers
	// This loads functions like vkCreateDevice, vkEnumeratePhysicalDevices, etc.
	vk::detail::defaultDispatchLoaderDynamic.init(instance);
}

void InitializeVulkanDispatcherDevice(vk::Device device)
{
	// Initialize device-level function pointers
	// This loads all device-specific functions including extension functions
	// like vkCmdBindDescriptorBuffersEXT, vkGetDescriptorEXT, etc.
	vk::detail::defaultDispatchLoaderDynamic.init(device);
}

