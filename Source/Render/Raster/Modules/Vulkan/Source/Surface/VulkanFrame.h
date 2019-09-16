#pragma once

#include "Render/Raster/Modules/Vulkan/VulkanFramebuffer.h"
#include "Render/Raster/Modules/Vulkan/VulkanSemaphore.h"
#include "Render/Raster/Modules/Vulkan/VulkanFence.h"

#include <optional>

class VulkanFrame{

public:

	VulkanFrame() = default;
	~VulkanFrame() = default;

	VulkanFrame(const VulkanFrame&) = delete;
	VulkanFrame(VulkanFrame&&) noexcept = default;

	VulkanFrame& operator=(const VulkanFrame&) = delete;
	VulkanFrame& operator=(VulkanFrame&&) noexcept = default;

	VulkanFrameBuffer& Framebuffer();
	VulkanSemaphore& RenderSemaphore();

private:
	
	std::optional<VulkanFrameBuffer> framebuffer_;
	std::optional<VulkanSemaphore> presentSemaphore_;
	std::optional<VulkanSemaphore> renderSemaphore_;
	std::optional<VulkanFence> imageFence_;
	
};
