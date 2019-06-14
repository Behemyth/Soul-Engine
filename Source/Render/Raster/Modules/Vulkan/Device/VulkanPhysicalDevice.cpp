#include "VulkanPhysicalDevice.h"

VulkanPhysicalDevice::VulkanPhysicalDevice(vk::Instance instance, vk::PhysicalDevice device):
	instance_(instance), device_(device)
{
}

const vk::PhysicalDevice& VulkanPhysicalDevice::Get()
{

	return device_;

}