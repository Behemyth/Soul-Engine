#pragma once

#include "Core/Structure/Span.h"
#include "VulkanShader.h"

#include <vulkan/vulkan.hpp>

class VulkanSubPass
{

public:

	explicit VulkanSubPass(nonstd::span<vk::AttachmentReference2KHR>);
	~VulkanSubPass() = default;

	VulkanSubPass(const VulkanSubPass&) = default;
	VulkanSubPass(VulkanSubPass&&) noexcept = default;

	VulkanSubPass& operator=(const VulkanSubPass&) = default;
	VulkanSubPass& operator=(VulkanSubPass&&) noexcept = default;

	[[nodiscard]] const vk::SubpassDescription2KHR& Description() const;

private:

	vk::SubpassDescription2KHR description_;
	
};
