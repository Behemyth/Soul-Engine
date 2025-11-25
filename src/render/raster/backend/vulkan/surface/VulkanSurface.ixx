export module synodic.soul.raster.backend.vulkan:surface;

import std;
import vulkan_hpp;

import :device;
import synodic.soul.scheduler;

export class VulkanSurface {

public:

	VulkanSurface(const vk::Instance& instance, const vk::SurfaceKHR& surface):
		instance_(instance),
		surface_(surface),
		format_ {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear}
	{
	}

	~VulkanSurface() {
		if (surface_) {
			instance_.destroySurfaceKHR(surface_);
		}
	}

	VulkanSurface(const VulkanSurface&) = delete;

	VulkanSurface(VulkanSurface&& other) noexcept :
		instance_(other.instance_),
		surface_(other.surface_),
		size_(other.size_),
		format_(other.format_)
	{
		other.surface_ = nullptr;
		other.instance_ = nullptr;
	}

	VulkanSurface& operator=(const VulkanSurface&) = delete;

	VulkanSurface& operator=(VulkanSurface&& other) noexcept {
		if (this != &other) {
			if (surface_) {
				instance_.destroySurfaceKHR(surface_);
			}
			instance_ = other.instance_;
			surface_ = other.surface_;
			size_ = other.size_;
			format_ = other.format_;
			other.surface_ = nullptr;
			other.instance_ = nullptr;
		}
		return *this;
	}

	[[nodiscard]] vk::SurfaceKHR Handle() const {
		return surface_;
	}

	template<SchedulerBackend SchedulerType>
	vk::SurfaceFormatKHR UpdateFormat(const VulkanDevice<SchedulerType>& device);

	[[nodiscard]] vk::SurfaceFormatKHR Format() const {
		return format_;
	}

private:

	vk::Instance instance_;
	vk::SurfaceKHR surface_;
	vk::Extent2D size_;

	vk::SurfaceFormatKHR format_;

};

// Template implementation
template<SchedulerBackend SchedulerType>
vk::SurfaceFormatKHR VulkanSurface::UpdateFormat(const VulkanDevice<SchedulerType>& device)
{

	const auto& physicalDevice = device.Physical();

	vk::PhysicalDeviceSurfaceInfo2KHR surfaceInfo;
	surfaceInfo.surface = surface_;

	const auto formats =
		physicalDevice.getSurfaceFormats2KHR(surfaceInfo);

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
