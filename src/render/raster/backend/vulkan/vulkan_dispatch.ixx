// Dynamic dispatch interface for Vulkan-Hpp.
// With VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1, the vulkan module uses a dynamic dispatcher
// that must be initialized at runtime. This module provides the storage and initialization.

export module synodic.soul.raster.backend.vulkan:dispatch;

import std;
import vulkan;

// Initialize the dynamic dispatcher (called before any Vulkan calls)
// This loads the minimal function pointers needed to create an instance
export void InitializeVulkanDispatcher();

// Initialize instance-level functions (called after instance creation)
export void InitializeVulkanDispatcherInstance(vk::Instance instance);

// Initialize device-level functions (called after device creation)
// This loads all device-specific functions including extension functions like VK_EXT_descriptor_buffer
export void InitializeVulkanDispatcherDevice(vk::Device device);

