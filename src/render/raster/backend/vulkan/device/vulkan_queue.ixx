export module synodic.soul.raster.backend.vulkan:queue;

import std;
import vulkan_hpp;

class VulkanQueue {

public:

	VulkanQueue(const vk::Device& device, std::uint32_t familyIndex, std::uint32_t index);
	~VulkanQueue() = default;

	VulkanQueue(const VulkanQueue&) = default;
	VulkanQueue(VulkanQueue&&) noexcept = default;

	VulkanQueue& operator=(const VulkanQueue&) = default;
	VulkanQueue& operator=(VulkanQueue&&) noexcept = default;

	bool Submit();
	[[nodiscard]] vk::Result Present(std::span<vk::Semaphore> semaphores,
		std::span<vk::SwapchainKHR> swapChains,
		std::span<std::uint32_t> imageIndices) const;

	[[nodiscard]]  const vk::Queue& Handle() const;
	[[nodiscard]] std::uint32_t FamilyIndex() const;

private:

	vk::Device device_;
	vk::Queue queue_;

	std::uint32_t familyIndex_;
	std::uint32_t index_;


};
