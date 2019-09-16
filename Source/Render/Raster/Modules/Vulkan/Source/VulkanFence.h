#pragma once

#include <vulkan/vulkan.hpp>

class VulkanFence
{

public:

	VulkanFence(vk::Device);
	~VulkanFence() = default;

	VulkanFence(const VulkanFence&) = delete;
	VulkanFence(VulkanFence&&) noexcept = default;

	VulkanFence& operator=(const VulkanFence&) = delete;
	VulkanFence& operator=(VulkanFence&&) noexcept = default;


private:

	vk::Device device_;
	
	vk::Fence fence_;

};
