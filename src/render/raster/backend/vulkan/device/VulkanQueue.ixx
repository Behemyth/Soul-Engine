module;

#include <vulkan/vulkan.hpp>

export module render.raster.vulkan:queue;

import synodic.soul.engine;

using std::uint32_t = std::uint32_t;

export class VulkanQueue {

public:

	VulkanQueue(const vk::Device& device, std::uint32_t familyIndex, std::uint32_t index);
	~VulkanQueue() = default;

	VulkanQueue(const VulkanQueue&) = default;
	VulkanQueue(VulkanQueue&&) noexcept = default;

	VulkanQueue& operator=(const VulkanQueue&) = default;
	VulkanQueue& operator=(VulkanQueue&&) noexcept = default;

	bool Submit();
	bool Present(nonstd::span<vk::Semaphore> semaphores,
		nonstd::span<vk::SwapchainKHR> swapChains,
		nonstd::span<std::uint32_t> imageIndices) const;

	[[nodiscard]]  const vk::Queue& Handle() const;
	[[nodiscard]] std::uint32_t FamilyIndex() const;

private:

	vk::Device device_;
	vk::Queue queue_;

	std::uint32_t familyIndex_;
	std::uint32_t index_;


};
