export module synodic.soul.raster.backend.vulkan:device;

import std;
import vulkan_hpp;

import :queue;
import :allocator;
import :error;
import synodic.soul.scheduler;
import synodic.soul.engine;

// TODO: VulkanDevice should inherit from RasterDevice but this causes
// MSVC Internal Compiler Error when importing synodic.soul.raster
// For now, VulkanDevice just implements the same interface
export template<typename SchedulerType> requires SchedulerBackend<SchedulerType>
class VulkanDevice final {

public:

	VulkanDevice(SchedulerType&,
		const vk::Instance&,
		const vk::PhysicalDevice&,
		std::span<std::string>,
		std::span<std::string>,
		std::uint32_t vulkanApiVersion);

	~VulkanDevice() {
		device_.destroy();
	}

	VulkanDevice(const VulkanDevice &) = delete;
	VulkanDevice(VulkanDevice &&) noexcept = default;

	VulkanDevice& operator=(const VulkanDevice &) = delete;
	VulkanDevice& operator=(VulkanDevice&&) noexcept = default;

	void Synchronize() {
		device_.waitIdle();
	}

	const vk::Device& Logical() const {
		return device_;
	}

	const vk::PhysicalDevice& Physical() const {
		return physicalDevice_;
	}

	VulkanAllocator& Allocator() noexcept { return allocator_; }
	const VulkanAllocator& Allocator() const noexcept { return allocator_; }

	bool SurfaceSupported(vk::SurfaceKHR&);
	VulkanResult<std::uint32_t> HighFamilyIndex() const;

	std::span<VulkanQueue> GraphicsQueues() {
		return {graphicsQueues_};
	}

	std::span<VulkanQueue> ComputeQueues() {
		return {computeQueues_};
	}

	std::span<VulkanQueue> TransferQueues() {
		return {transferQueues_};
	}

private:

	SchedulerType& scheduler_;

	vk::Device device_;
	vk::PhysicalDevice physicalDevice_;

	std::vector<VulkanQueue> graphicsQueues_;
	std::vector<VulkanQueue> computeQueues_;
	std::vector<VulkanQueue> transferQueues_;

	VulkanAllocator allocator_;

};

// Template implementation
template<typename SchedulerType> requires SchedulerBackend<SchedulerType>
VulkanDevice<SchedulerType>::VulkanDevice(SchedulerType& scheduler,
	const vk::Instance& instance,
	const vk::PhysicalDevice& physicalDevice,
	std::span<std::string> validationLayers,
	std::span<std::string> requiredExtensions,
	std::uint32_t vulkanApiVersion):
	scheduler_(scheduler),
	physicalDevice_(physicalDevice),
	allocator_(instance, physicalDevice, vk::Device(), vulkanApiVersion)  // Temporary, will be updated
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

	std::vector<std::pair<std::uint32_t, std::uint32_t>> transferIndices;
	std::vector<std::pair<std::uint32_t, std::uint32_t>> computeIndices;
	std::vector<std::pair<std::uint32_t, std::uint32_t>> graphicsIndices;

	std::vector<std::uint32_t> familyQueuesUsed(queueFamilyProperties.size(), 0);

	std::uint32_t maxThreads = std::thread::hardware_concurrency();

	// Iterate over all the queue families prioritizing dedicated hardware
	for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(queueFamilyProperties.size()); ++i) {

		auto& queueFamily = queueFamilyProperties[i].queueFamilyProperties;

		std::uint32_t count = (std::min)(queueFamily.queueCount, maxThreads);
		familyQueuesUsed[i] += count;

		// Transfer
		if ((queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) &&
			!(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
			!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)) {

			transferIndices.reserve(count);
			for (std::uint32_t t = 0; t < count; ++t) {

				transferIndices.emplace_back(i,t);

			}

		}

		// Compute
		else if ((queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
				!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))  {

			computeIndices.reserve(count);
			for (std::uint32_t t = 0; t < count; ++t) {

				computeIndices.emplace_back(i, t);
			}

		}

		// Graphics
		else if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
		{

			graphicsIndices.reserve(count);
			for (std::uint32_t t = 0; t < count; ++t) {

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


	for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(queueFamilyProperties.size()); ++i) {

		//TODO: calculate priorities alongside Queue selection
		priorities[i].resize(familyQueuesUsed[i], 1.0f);

		vk::DeviceQueueCreateInfo deviceQueueCreateInfo;
		deviceQueueCreateInfo.flags = vk::DeviceQueueCreateFlags();
		deviceQueueCreateInfo.queueFamilyIndex = static_cast<std::uint32_t>(i);
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

	// Now initialize the allocator with the created device
	allocator_ = VulkanAllocator(instance, physicalDevice_, device_, vulkanApiVersion);
}

template<typename SchedulerType>
	requires SchedulerBackend<SchedulerType>
VulkanDevice<SchedulerType>::~VulkanDevice()
{

	device_.destroy();

}

template<typename SchedulerType>
	requires SchedulerBackend<SchedulerType>
void VulkanDevice<SchedulerType>::Synchronize()
{

	device_.waitIdle();

}

template<typename SchedulerType>
	requires SchedulerBackend<SchedulerType>
const vk::Device& VulkanDevice<SchedulerType>::Logical() const
{

	return device_;

}

template<typename SchedulerType>
	requires SchedulerBackend<SchedulerType>
const vk::PhysicalDevice& VulkanDevice<SchedulerType>::Physical() const
{

	return physicalDevice_;

}

template<typename SchedulerType>
	requires SchedulerBackend<SchedulerType>
bool VulkanDevice<SchedulerType>::SurfaceSupported(vk::SurfaceKHR& surface)
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

template<typename SchedulerType>
	requires SchedulerBackend<SchedulerType>
VulkanResult<std::uint32_t> VulkanDevice<SchedulerType>::HighFamilyIndex() const
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

	return std::unexpected(VulkanError::DeviceNotFound);

}

template<typename SchedulerType>
	requires SchedulerBackend<SchedulerType>
std::span<VulkanQueue> VulkanDevice<SchedulerType>::GraphicsQueues()
{
	return {graphicsQueues_};
}

template<typename SchedulerType>
	requires SchedulerBackend<SchedulerType>
std::span<VulkanQueue> VulkanDevice<SchedulerType>::ComputeQueues()
{
	return {computeQueues_};
}

template<typename SchedulerType>
	requires SchedulerBackend<SchedulerType>
std::span<VulkanQueue> VulkanDevice<SchedulerType>::TransferQueues()
{
	return {transferQueues_};
}

