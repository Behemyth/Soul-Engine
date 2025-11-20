module;

#include <vulkan/vulkan.hpp>

export module render.raster.vulkan:swapchain;

import synodic.soul.engine;
import :pipeline;
import :device;
import :surface;

import <vector>;

using std::uint32_t = std::uint32_t;

export class VulkanSwapChain : Component {

public:

	VulkanSwapChain(VulkanDevice&,
		VulkanSurface&,
		bool,
		VulkanSwapChain* = nullptr);
	~VulkanSwapChain();

	VulkanSwapChain(const VulkanSwapChain&) = delete;
	VulkanSwapChain(VulkanSwapChain&& o) noexcept = default;

	VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;
	VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept = default;

	nonstd::span<vk::Image> Images();
	nonstd::span<vk::ImageView> ImageViews();
	[[nodiscard]] std::uint32_t ActiveImageIndex() const;

	void AcquireImage(const vk::Semaphore&);

	[[nodiscard]] const vk::Device& Device() const;
	[[nodiscard]] vk::Extent2D Size() const;
	[[nodiscard]] vk::SwapchainKHR Handle() const;


private:

	vk::Device device_;

	std::vector<vk::Image> renderImages_;
	std::vector<vk::ImageView> renderImageViews_;

	std::uint32_t activeImageIndex_;

	vk::Extent2D size_;
	vk::SwapchainKHR swapChain_;


};
