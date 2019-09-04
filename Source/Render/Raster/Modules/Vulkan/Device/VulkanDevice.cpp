#include "VulkanDevice.h"

#include "Parallelism/Scheduler/TaskParameters.h"
#include "Parallelism/Scheduler/SchedulerModule.h"
#include "Core/System/Compiler.h"
#include "Core/Utility/Exception/Exception.h"

#include <thread>
#include <algorithm>

VulkanDevice::VulkanDevice(std::shared_ptr<SchedulerModule>& scheduler,
	const vk::Instance& instance,
	const vk::PhysicalDevice& physicalDevice,
	nonstd::span<std::string> validationLayers,
	nonstd::span<std::string> requiredExtensions):
	scheduler_(scheduler),
	physicalDevice_(physicalDevice)
{

	// Convert strings to c-strings
	std::vector<const char*> cValidationLayers;
	cValidationLayers.reserve(validationLayers.size());
	for (auto& layer : validationLayers) {

		cValidationLayers.push_back(layer.c_str());
	}

	std::vector<const char*> cExtensions;
	cExtensions.reserve(requiredExtensions.size());
	for (auto& extension : requiredExtensions) {

		cExtensions.push_back(extension.c_str());
	}

	// TODO: Validate validation layers
	if constexpr (Compiler::Debug()) {

		std::vector<vk::LayerProperties> availableLayers =
			physicalDevice.enumerateDeviceLayerProperties();
	}

	// TODO: Validate extensions
	std::vector<vk::ExtensionProperties> availableExtensions =
		physicalDevice.enumerateDeviceExtensionProperties();

	// TODO: Query properties
	vk::PhysicalDeviceProperties deviceProperties_ = physicalDevice_.getProperties();
	vk::PhysicalDeviceFeatures deviceFeatures_ = physicalDevice_.getFeatures();
	vk::PhysicalDeviceMemoryProperties memoryProperties_ = physicalDevice_.getMemoryProperties();

	// Start queue selection
	std::vector<vk::QueueFamilyProperties2> queueFamilyProperties =
		physicalDevice.getQueueFamilyProperties2();

	std::vector<std::pair<uint, uint>> transferIndices;
	std::vector<std::pair<uint, uint>> computeIndices;
	std::vector<std::pair<uint, uint>> graphicsIndices;

	std::vector<uint> familyQueuesUsed(queueFamilyProperties.size(), 0);

	uint maxThreads = std::thread::hardware_concurrency();

	// Iterate over all the queue families prioritizing dedicated hardware
	for (uint i = 0; i < static_cast<uint>(queueFamilyProperties.size()); ++i) {

		auto& queueFamily = queueFamilyProperties[i].queueFamilyProperties;

		uint count = (std::min)(queueFamily.queueCount, maxThreads);
		familyQueuesUsed[i] += count;

		// Transfer
		if ((queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) &&
			!(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
			!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)) {

			transferIndices.reserve(count);
			for (uint t = 0; t < count; ++t) {

				transferIndices.emplace_back(i,t);

			}

		}

		// Compute
		else if ((queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
				!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))  {

			computeIndices.reserve(count);
			for (uint t = 0; t < count; ++t) {

				computeIndices.emplace_back(i, t);
			}

		}

		// Graphics
		else if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
		{

			graphicsIndices.reserve(count);
			for (uint t = 0; t < count; ++t) {

				graphicsIndices.emplace_back(i, t);

			}

		}
	}

	//TODO: use remaining queues to supplement other queue types


	//Create the info structures
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::vector<std::vector<float>> priorities;
	queueCreateInfos.resize(queueFamilyProperties.size());
	priorities.resize(queueFamilyProperties.size());


	for (uint i = 0; i < static_cast<uint>(queueFamilyProperties.size()); ++i) {

		//TODO: calculate priorities alongside Queue selection
		priorities[i].resize(familyQueuesUsed[i], 1.0f);

		vk::DeviceQueueCreateInfo deviceQueueCreateInfo;
		deviceQueueCreateInfo.flags = vk::DeviceQueueCreateFlags();
		deviceQueueCreateInfo.queueFamilyIndex = static_cast<uint>(i);
		deviceQueueCreateInfo.queueCount = familyQueuesUsed[i];
		deviceQueueCreateInfo.pQueuePriorities = priorities[i].data();

		queueCreateInfos[i] = (deviceQueueCreateInfo);
	}

	//Create the device
	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.flags = vk::DeviceCreateFlags();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(cExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = cExtensions.data();
	deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(cValidationLayers.size());
	deviceCreateInfo.ppEnabledLayerNames = cValidationLayers.data();

	device_ = physicalDevice.createDevice(deviceCreateInfo);

	dispatcher_ = vk::DispatchLoaderDynamic(instance, device_);

	//Device is created, queues can be retrieved
	for (auto& indices : transferIndices) {
		transferQueues_.emplace_back(device_, indices.first, indices.second);
	}

	for (auto& indices : computeIndices) {
		computeQueues_.emplace_back(device_, indices.first, indices.second);
	}

	for (auto& indices : graphicsIndices) {
		graphicsQueues_.emplace_back(device_, indices.first, indices.second);
	}
}

VulkanDevice::~VulkanDevice()
{

	device_.destroy();

}

void VulkanDevice::Synchronize()
{

	device_.waitIdle();

}

const vk::Device& VulkanDevice::Logical() const
{

	return device_;

}

const vk::PhysicalDevice& VulkanDevice::Physical() const
{

	return physicalDevice_;

}

const vk::DispatchLoaderDynamic& VulkanDevice::DispatchLoader() const
{

	return dispatcher_;

}

bool VulkanDevice::SurfaceSupported(vk::SurfaceKHR& surface)
{

	bool supported = true;
	for (auto& graphicsQueue : graphicsQueues_) {
	
		if (!physicalDevice_.getSurfaceSupportKHR(
		graphicsQueue.FamilyIndex(),
		surface)) {

			return false;

		}

	}

	return supported;

}

uint VulkanDevice::HighFamilyIndex() const
{

	if (!graphicsQueues_.empty()) {
		return graphicsQueues_[0].FamilyIndex();
	}

	if (!computeQueues_.empty()) {
		return computeQueues_[0].FamilyIndex();
	}

	if (!transferQueues_.empty()) {
		return transferQueues_[0].FamilyIndex();
	}

	throw NotImplemented();

}

nonstd::span<VulkanQueue> VulkanDevice::GraphicsQueues()
{
	return {graphicsQueues_};
}

nonstd::span<VulkanQueue> VulkanDevice::ComputeQueues()
{
	return {computeQueues_};
}

nonstd::span<VulkanQueue> VulkanDevice::TransferQueues()
{
	return {transferQueues_};
}