module synodic.soul.raster.backend.vulkan;

import std;

VulkanSemaphore::VulkanSemaphore(vk::Device device) :
	device_(device)
{

	vk::SemaphoreCreateInfo semaphoreInfo;

	semaphore_ = device.createSemaphore(semaphoreInfo);

}

vk::Semaphore VulkanSemaphore::Handle() const
{

	return semaphore_;

}

// Timeline Semaphore implementation

VulkanTimelineSemaphore::VulkanTimelineSemaphore(vk::Device device, std::uint64_t initialValue) :
	device_(device)
{
	vk::SemaphoreTypeCreateInfo timelineInfo;
	timelineInfo.semaphoreType = vk::SemaphoreType::eTimeline;
	timelineInfo.initialValue = initialValue;

	vk::SemaphoreCreateInfo semaphoreInfo;
	semaphoreInfo.pNext = &timelineInfo;

	semaphore_ = device.createSemaphore(semaphoreInfo);
}

vk::Semaphore VulkanTimelineSemaphore::Handle() const
{
	return semaphore_;
}

void VulkanTimelineSemaphore::Wait(std::uint64_t value) const
{
	vk::SemaphoreWaitInfo waitInfo;
	waitInfo.semaphoreCount = 1;
	waitInfo.pSemaphores = &semaphore_;
	waitInfo.pValues = &value;

	// Wait indefinitely
	[[maybe_unused]] auto result = device_.waitSemaphores(waitInfo, std::numeric_limits<std::uint64_t>::max());
}

void VulkanTimelineSemaphore::Signal(std::uint64_t value) const
{
	vk::SemaphoreSignalInfo signalInfo;
	signalInfo.semaphore = semaphore_;
	signalInfo.value = value;

	device_.signalSemaphore(signalInfo);
}

std::uint64_t VulkanTimelineSemaphore::GetValue() const
{
	return device_.getSemaphoreCounterValue(semaphore_);
}
