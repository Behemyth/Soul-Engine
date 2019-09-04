#pragma once

#include "Types.h"
#include "Core/Structure/Span.h"

#include <vulkan/vulkan.hpp>


class VulkanQueue {

public:

	VulkanQueue(const vk::Device& device, uint familyIndex, uint index);
	~VulkanQueue() = default;

	VulkanQueue(const VulkanQueue&) = default;
	VulkanQueue(VulkanQueue&&) noexcept = default;

	VulkanQueue& operator=(const VulkanQueue&) = default;
	VulkanQueue& operator=(VulkanQueue&&) noexcept = default;

	bool Submit();
	bool Present(nonstd::span<vk::Semaphore> semaphores,
		nonstd::span<vk::SwapchainKHR> swapChains,
		nonstd::span<uint> imageIndices) const;

	[[nodiscard]]  const vk::Queue& Handle() const;
	[[nodiscard]] uint FamilyIndex() const;

private:

	vk::Device device_;
	vk::Queue queue_;

	uint familyIndex_;
	uint index_;


};
