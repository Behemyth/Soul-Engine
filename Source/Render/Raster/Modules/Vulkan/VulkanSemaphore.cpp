#include "VulkanSemaphore.h"

VulkanSemaphore::VulkanSemaphore(vk::Device device) :
	device_(device)
{
	
	vk::SemaphoreCreateInfo semaphoreInfo;

	semaphore_ = device.createSemaphore(semaphoreInfo);
	
}