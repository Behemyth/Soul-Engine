export module synodic.soul.raster.backend.vulkan:swapchain;


import std;
import vulkan_hpp;
import :device;
import :surface;
import :error;

export class VulkanSwapChain {

public:

	// Factory method that returns expected for proper error handling
	static VulkanResult<VulkanSwapChain> Create(VulkanDevice&,
		VulkanSurface&,
		bool,
		VulkanSwapChain* = nullptr);

	VulkanSwapChain(VulkanSwapChain&& o) noexcept = default;
	VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept = default;
	~VulkanSwapChain();

	VulkanSwapChain(const VulkanSwapChain&) = delete;
	VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

	std::span<vk::Image> Images();
	std::span<vk::ImageView> ImageViews();
	[[nodiscard]] std::uint32_t ActiveImageIndex() const;

	VulkanResult<std::uint32_t> AcquireImage(const vk::Semaphore&);

	[[nodiscard]] const vk::Device& Device() const;
	[[nodiscard]] vk::Extent2D Size() const;
	[[nodiscard]] vk::SwapchainKHR Handle() const;


private:

	// Private constructor for factory method
	VulkanSwapChain() = default;

	vk::Device device_;

	std::vector<vk::Image> renderImages_;
	std::vector<vk::ImageView> renderImageViews_;

	std::uint32_t activeImageIndex_;

	vk::Extent2D size_;
	vk::SwapchainKHR swapChain_;


};
