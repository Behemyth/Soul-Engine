module render.raster.vulkan;

VulkanPhysicalDevice::VulkanPhysicalDevice(vk::Instance instance, vk::PhysicalDevice device):
	instance_(instance), device_(device)
{
}

const vk::PhysicalDevice& VulkanPhysicalDevice::Handle()
{

	return device_;

}
