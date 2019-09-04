#pragma once

#include "Render/Raster/Modules/Vulkan/Pipeline/VulkanPipeline.h"
#include "Core/Composition/Component/Component.h"

#include <vulkan/vulkan.hpp>

class VulkanDevice;
class VulkanSurface;

class VulkanSwapChain : Component {

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
	[[nodiscard]] uint ActiveImageIndex() const;

	void AcquireImage(const vk::Semaphore&);

	[[nodiscard]] const vk::Device& Device() const;
	[[nodiscard]] vk::Extent2D Size() const;
	[[nodiscard]] vk::SwapchainKHR Handle() const;

	
private:

	vk::Device device_;

	std::vector<vk::Image> renderImages_;
	std::vector<vk::ImageView> renderImageViews_;

	uint activeImageIndex_;

	vk::Extent2D size_;
	vk::SwapchainKHR swapChain_;


};
