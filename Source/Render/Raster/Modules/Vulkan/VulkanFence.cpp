#include "VulkanFence.h"

VulkanFence::VulkanFence(vk::Device device):
	device_(device)
{
	
	vk::FenceCreateInfo fenceInfo;
	fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
	
	fence_ = device.createFence(fenceInfo);
	
}