#include "VulkanSurface.h"

VulkanSurface::VulkanSurface(const vk::Instance& instance, const vk::SurfaceKHR& surface): 
	instance_(instance),
	surface_(surface), 
	format_ {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear}
{
}

VulkanSurface::~VulkanSurface()
{

	instance_.destroySurfaceKHR(surface_);

}

vk::SurfaceKHR VulkanSurface::Handle() const
{

	return surface_;

}

vk::SurfaceFormatKHR VulkanSurface::UpdateFormat(const VulkanDevice& device)
{

	const auto& physicalDevice = device.Physical();

	vk::PhysicalDeviceSurfaceInfo2KHR surfaceInfo;
	surfaceInfo.surface = surface_;

	const auto formats =
		physicalDevice.getSurfaceFormats2KHR(surfaceInfo, device.DispatchLoader());

	// TODO: pick formats better
	if (!formats.empty() && formats.front().surfaceFormat.format == vk::Format::eUndefined) {
		return format_;
	}

	for (const auto& format : formats) {

		if (format.surfaceFormat.format == vk::Format::eB8G8R8A8Unorm &&
			format.surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return format_;
		}
	}

	format_.format = formats.front().surfaceFormat.format;
	format_.colorSpace = formats.front().surfaceFormat.colorSpace;

	return format_;
}

vk::SurfaceFormatKHR VulkanSurface::Format() const
{

	return format_;

}