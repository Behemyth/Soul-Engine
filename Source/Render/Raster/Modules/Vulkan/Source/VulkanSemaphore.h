#pragma once

#include <vulkan/vulkan.hpp>

class VulkanSemaphore
{

public:

	VulkanSemaphore(vk::Device);
	~VulkanSemaphore() = default;

	VulkanSemaphore(const VulkanSemaphore&) = delete;
	VulkanSemaphore(VulkanSemaphore&&) noexcept = default;

	VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;
	VulkanSemaphore& operator=(VulkanSemaphore&&) noexcept = default;

	[[nodiscard]] vk::Semaphore Handle() const;
	
private:
	
	vk::Device device_;
	
	vk::Semaphore semaphore_;

};
