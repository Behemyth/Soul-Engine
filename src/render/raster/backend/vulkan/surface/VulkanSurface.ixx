export module render.raster.vulkan:surface;

import std;
import vulkan_hpp;

import :device;

export class VulkanSurface {

public:

	VulkanSurface(const vk::Instance&, const vk::SurfaceKHR&);
	~VulkanSurface();

	VulkanSurface(const VulkanSurface&) = delete;
	VulkanSurface(VulkanSurface&&) noexcept = default;

	VulkanSurface& operator=(const VulkanSurface&) = delete;
	VulkanSurface& operator=(VulkanSurface&&) noexcept = default;

	[[nodiscard]] vk::SurfaceKHR Handle() const;

	[[nodiscard]] vk::SurfaceFormatKHR UpdateFormat(const VulkanDevice& device);
	[[nodiscard]] vk::SurfaceFormatKHR Format() const;

private:

	vk::Instance instance_;
	vk::SurfaceKHR surface_;
	vk::Extent2D size_;

	vk::SurfaceFormatKHR format_;

};
