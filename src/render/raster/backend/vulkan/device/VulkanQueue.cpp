module synodic.soul.raster.backend.vulkan;

VulkanQueue::VulkanQueue(const vk::Device& device, std::uint32_t familyIndex, std::uint32_t index):
	device_(device), familyIndex_(familyIndex), index_(index)
{

	queue_ = device_.getQueue(familyIndex_, index_);
}

bool VulkanQueue::Submit()
{

	return false;

}

vk::Result VulkanQueue::Present(
	std::span<vk::Semaphore> semaphores,
	std::span<vk::SwapchainKHR> swapChains,
	std::span<std::uint32_t> imageIndices) const
{

	// Verify sizes match
	if (swapChains.size() != imageIndices.size()) {
		return vk::Result::eErrorUnknown;
	}

	vk::PresentInfoKHR presentInfo;
	presentInfo.waitSemaphoreCount = static_cast<std::uint32_t>(semaphores.size());
	presentInfo.pWaitSemaphores = semaphores.data();
	presentInfo.swapchainCount = static_cast<std::uint32_t>(swapChains.size());
	presentInfo.pSwapchains = swapChains.data();
	presentInfo.pImageIndices = imageIndices.data();
	presentInfo.pResults = nullptr;

	// Use the non-throwing version by catching and returning the result
	try {
		return queue_.presentKHR(presentInfo);
	} catch (const vk::OutOfDateKHRError&) {
		return vk::Result::eErrorOutOfDateKHR;
	} catch (const vk::SurfaceLostKHRError&) {
		return vk::Result::eErrorSurfaceLostKHR;
	}
}

const vk::Queue& VulkanQueue::Handle() const
{

	return queue_;
}

std::uint32_t VulkanQueue::FamilyIndex() const
{

	return familyIndex_;
}
