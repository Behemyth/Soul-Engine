#include "VulkanSwapChain.h"

#include "Render/Raster/Modules/Vulkan/Device/VulkanDevice.h"
#include "Core/Utility/Exception/Exception.h"

#include "Core/Geometry/Vertex.h"
#include "Transput/Resource/Resource.h"
#include "Render/Raster/Modules/Vulkan/Surface/VulkanSurface.h"


VulkanSwapChain::VulkanSwapChain(VulkanDevice& device,
	VulkanSurface& surface,
	bool vSync,
	VulkanSwapChain* oldSwapChain):
	device_(device.Logical()),
	activeImageIndex_(0)
{
	auto& logicalDevice = device.Logical();
	const auto& physicalDevice = device.Physical();

	const vk::SurfaceCapabilitiesKHR surfaceCapabilities =
		physicalDevice.getSurfaceCapabilitiesKHR(surface.Handle());
	std::vector<vk::PresentModeKHR> presentModes =
		physicalDevice.getSurfacePresentModesKHR(surface.Handle());

	assert(!presentModes.empty());

	vk::Extent2D swapChainSize;
	if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
		swapChainSize = size_;
	}
	else {
		swapChainSize = surfaceCapabilities.currentExtent;
	}

	size_ = swapChainSize;

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
		
		throw NotImplemented();
		
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
	assert(device.SurfaceSupported(surfaceHandle));

	swapChain_ = device_.createSwapchainKHR(swapChainCreateInfo);
	renderImages_ = device_.getSwapchainImagesKHR(swapChain_);
	renderImageViews_.resize(renderImages_.size());

	// Set up synchronization primitives
	vk::SemaphoreCreateInfo semaphoreInfo;

	vk::FenceCreateInfo fenceInfo;
	fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
	
	for (auto i = 0; i < renderImages_.size(); ++i) {

		vk::ImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.flags = vk::ImageViewCreateFlags();
		imageViewCreateInfo.image = renderImages_[i];
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
		imageViewCreateInfo.format = format.format;
		imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		renderImageViews_[i] = logicalDevice.createImageView(imageViewCreateInfo);
		
	}

}

VulkanSwapChain::~VulkanSwapChain()
{

	for (auto i = 0; i < renderImageViews_.size(); ++i) {
		
		device_.destroyImageView(renderImageViews_[i]);
		
	}

	device_.waitIdle();
	device_.destroySwapchainKHR(swapChain_);

}

nonstd::span<vk::Image> VulkanSwapChain::Images()
{

	return {renderImages_};

}

nonstd::span<vk::ImageView> VulkanSwapChain::ImageViews()
{

	return {renderImageViews_};

}

uint VulkanSwapChain::ActiveImageIndex() const
{

	return activeImageIndex_;

}

void VulkanSwapChain::AcquireImage(const vk::Semaphore& presentSemaphore)
{

	auto [acquireResult, activeImageIndex_] = device_.acquireNextImageKHR(swapChain_, std::numeric_limits<uint64_t>::max(), presentSemaphore, nullptr);

	if (acquireResult != vk::Result::eSuccess) {

		throw NotImplemented();

	}

}

const vk::Device& VulkanSwapChain::Device() const
{
	
	return device_;
	
}

vk::Extent2D VulkanSwapChain::Size() const
{
	return size_;
}

vk::SwapchainKHR VulkanSwapChain::Handle() const
{
	
	return swapChain_;
	
}