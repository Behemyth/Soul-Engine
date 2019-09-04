#pragma once

#include "Render/Raster/Modules/Vulkan/Device/VulkanDevice.h"

#include <vulkan/vulkan.hpp>

class VulkanSurface {

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
