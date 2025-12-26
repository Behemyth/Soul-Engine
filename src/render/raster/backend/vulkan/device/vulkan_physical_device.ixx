export module synodic.soul.raster.backend.vulkan:physical_device;

import std;
import vulkan;

export class VulkanPhysicalDevice {

public:

	VulkanPhysicalDevice(vk::Instance, vk::PhysicalDevice);
	~VulkanPhysicalDevice() = default;

	VulkanPhysicalDevice(const VulkanPhysicalDevice&) = default;
	VulkanPhysicalDevice(VulkanPhysicalDevice&&) noexcept = default;

	VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice&) = default;
	VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&&) noexcept = default;

	const vk::PhysicalDevice& Handle();

private:

	vk::Instance instance_;
	vk::PhysicalDevice device_;

};
