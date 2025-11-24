export module synodic.soul.raster.backend.vulkan:swapchain;


import std;
import vulkan_hpp;
import :device;
import :surface;
import :error;
import synodic.soul.scheduler;

export template<SchedulerBackend SchedulerType>
class VulkanSwapChain {

public:

	// Factory method that returns expected for proper error handling
	static VulkanResult<VulkanSwapChain> Create(VulkanDevice<SchedulerType>&,
		VulkanSurface&,
		bool,
		VulkanSwapChain* = nullptr);

	VulkanSwapChain(VulkanSwapChain&& o) noexcept = default;
	VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept = default;

	~VulkanSwapChain() {
		for (auto i = 0; i < renderImageViews_.size(); ++i) {
			device_.destroyImageView(renderImageViews_[i]);
		}
		device_.waitIdle();
		device_.destroySwapchainKHR(swapChain_);
	}

	VulkanSwapChain(const VulkanSwapChain&) = delete;
	VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

	std::span<vk::Image> Images() {
		return {renderImages_};
	}

	std::span<vk::ImageView> ImageViews() {
		return {renderImageViews_};
	}

	[[nodiscard]] std::uint32_t ActiveImageIndex() const {
		return activeImageIndex_;
	}

	VulkanResult<std::uint32_t> AcquireImage(const vk::Semaphore&);

	[[nodiscard]] const vk::Device& Device() const {
		return device_;
	}

	[[nodiscard]] vk::Extent2D Size() const {
		return size_;
	}

	[[nodiscard]] vk::SwapchainKHR Handle() const {
		return swapChain_;
	}


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

// Template implementation
template<SchedulerBackend SchedulerType>
VulkanResult<VulkanSwapChain<SchedulerType>> VulkanSwapChain<SchedulerType>::Create(VulkanDevice<SchedulerType>& device,
	VulkanSurface& surface,
	bool vSync,
	VulkanSwapChain* oldSwapChain)
{
	VulkanSwapChain swapChain;
	swapChain.device_ = device.Logical();
	swapChain.activeImageIndex_ = 0;

	auto& logicalDevice = device.Logical();
	const auto& physicalDevice = device.Physical();

	const vk::SurfaceCapabilitiesKHR surfaceCapabilities =
		physicalDevice.getSurfaceCapabilitiesKHR(surface.Handle());
	std::vector<vk::PresentModeKHR> presentModes =
		physicalDevice.getSurfacePresentModesKHR(surface.Handle());

	if (presentModes.empty()) {
		return std::unexpected(VulkanError::NoPresentModesAvailable);
	}

	vk::Extent2D swapChainSize;
	if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
		swapChainSize = swapChain.size_;
	}
	else {
		swapChainSize = surfaceCapabilities.currentExtent;
	}

	swapChain.size_ = swapChainSize;

	vk::PresentModeKHR swapChainPresentMode = vk::PresentModeKHR::eFifo;

	if (!vSync) {

		for (const auto& presentMode : presentModes) {
			if (presentMode == vk::PresentModeKHR::eMailbox) {
				swapChainPresentMode = vk::PresentModeKHR::eMailbox;
				break;
			}
			if (swapChainPresentMode != vk::PresentModeKHR::eMailbox &&
				presentMode == vk::PresentModeKHR::eImmediate) {
				swapChainPresentMode = vk::PresentModeKHR::eImmediate;
			}
		}
	}
	else {

		return std::unexpected(VulkanError::NotImplemented);

	}

	vk::SurfaceTransformFlagBitsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
		preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	}
	else {
		preTransform = surfaceCapabilities.currentTransform;
	}

	uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
		imageCount = surfaceCapabilities.maxImageCount;
	}

	vk::SurfaceFormatKHR format = surface.Format();

	vk::SwapchainCreateInfoKHR swapChainCreateInfo;
	swapChainCreateInfo.surface = surface.Handle();
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageFormat = format.format;
	swapChainCreateInfo.imageColorSpace = format.colorSpace;
	swapChainCreateInfo.imageExtent = swapChainSize;
	swapChainCreateInfo.imageUsage =
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
	swapChainCreateInfo.preTransform = preTransform;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
	swapChainCreateInfo.queueFamilyIndexCount = 0;
	swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	swapChainCreateInfo.presentMode = swapChainPresentMode;
	swapChainCreateInfo.clipped = true;
	swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapChainCreateInfo.oldSwapchain = oldSwapChain ? oldSwapChain->swapChain_ : nullptr;

	auto surfaceHandle = surface.Handle();
	if (!device.SurfaceSupported(surfaceHandle)) {
		return std::unexpected(VulkanError::SurfaceNotSupported);
	}

	swapChain.swapChain_ = swapChain.device_.createSwapchainKHR(swapChainCreateInfo);
	swapChain.renderImages_ = swapChain.device_.getSwapchainImagesKHR(swapChain.swapChain_);
	swapChain.renderImageViews_.resize(swapChain.renderImages_.size());

	// Set up synchronization primitives
	vk::SemaphoreCreateInfo semaphoreInfo;

	vk::FenceCreateInfo fenceInfo;
	fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	for (auto i = 0; i < swapChain.renderImages_.size(); ++i) {

		vk::ImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.flags = vk::ImageViewCreateFlags();
		imageViewCreateInfo.image = swapChain.renderImages_[i];
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
		imageViewCreateInfo.format = format.format;
		imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		swapChain.renderImageViews_[i] = logicalDevice.createImageView(imageViewCreateInfo);

	}

	return swapChain;
}

template<SchedulerBackend SchedulerType>
VulkanResult<std::uint32_t> VulkanSwapChain<SchedulerType>::AcquireImage(const vk::Semaphore& presentSemaphore)
{

	auto [acquireResult, imageIndex] = device_.acquireNextImageKHR(swapChain_, std::numeric_limits<uint64_t>::max(), presentSemaphore, nullptr);

	if (acquireResult != vk::Result::eSuccess) {
		return std::unexpected(FromVkResult(acquireResult));
	}

	activeImageIndex_ = imageIndex;
	return imageIndex;

}
