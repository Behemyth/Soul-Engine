#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/vec2.hpp>
#include "Core/Structure/Span.h"

class VulkanRenderPass;

class VulkanFrameBuffer{

public:

	VulkanFrameBuffer(const vk::Device& device,
		nonstd::span<vk::ImageView>,
		VulkanRenderPass&,
		vk::Extent2D&);
	~VulkanFrameBuffer();

	VulkanFrameBuffer(const VulkanFrameBuffer&) = delete;
	VulkanFrameBuffer(VulkanFrameBuffer&&) noexcept = default;

	VulkanFrameBuffer& operator=(const VulkanFrameBuffer&) = delete;
	VulkanFrameBuffer& operator=(VulkanFrameBuffer&&) noexcept = default;

	const vk::Framebuffer& Handle() const;

private:

	vk::Device device_;
	vk::Framebuffer frameBuffer_;


};
