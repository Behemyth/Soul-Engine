module render.raster.vulkan;

VulkanQueue::VulkanQueue(const vk::Device& device, std::uint32_t familyIndex, std::uint32_t index):
	device_(device), familyIndex_(familyIndex), index_(index)
{

	queue_ = device_.getQueue(familyIndex_, index_);
}

bool VulkanQueue::Submit()
{

	return false;

}

bool VulkanQueue::Present(
	std::span<vk::Semaphore> semaphores,
	std::span<vk::SwapchainKHR> swapChains,
	std::span<std::uint32_t> imageIndices) const
{

	// Verify sizes match
	if (swapChains.size() != imageIndices.size()) {
		throw std::runtime_error("SwapChain and imageIndices size mismatch");
	}

	//TODO: Reduce allocation calls
	std::vector<vk::Result> swapChainResults(swapChains.size());

	vk::PresentInfoKHR presentInfo;
	presentInfo.waitSemaphoreCount = semaphores.size();
	presentInfo.pWaitSemaphores = semaphores.data();
	presentInfo.swapchainCount = swapChains.size();
	presentInfo.pSwapchains = swapChains.data();
	presentInfo.pImageIndices = imageIndices.data();
	presentInfo.pResults = swapChainResults.data();

	const auto result = queue_.presentKHR(presentInfo);

	bool success = result == vk::Result::eSuccess;

	for (const auto& swapChainResult : swapChainResults) {

		success &= swapChainResult == vk::Result::eSuccess;

	}

	return success;

}

const vk::Queue& VulkanQueue::Handle() const
{

	return queue_;
}

std::uint32_t VulkanQueue::FamilyIndex() const
{

	return familyIndex_;
}
