#include "VulkanQueue.h"

VulkanQueue::VulkanQueue(const vk::Device& device, uint familyIndex, uint index):
	device_(device), familyIndex_(familyIndex), index_(index)
{

	queue_ = device_.getQueue(familyIndex_, index_);
}

bool VulkanQueue::Submit()
{

	return false;
	
}

bool VulkanQueue::Present(
	nonstd::span<vk::Semaphore> semaphores,
	nonstd::span<vk::SwapchainKHR> swapChains,
	nonstd::span<uint> imageIndices) const
{

	assert(swapChains.size() == imageIndices.size());

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

uint VulkanQueue::FamilyIndex() const
{

	return familyIndex_;
}