export module synodic.soul.raster.backend.vulkan:device;

import std;
import vulkan_hpp;

import :queue;
import :allocator;
import :error;
import synodic.soul.scheduler;
import synodic.soul.engine;

export template<SchedulerBackend SchedulerType>
class VulkanDevice final
{
private:
	// Internal builder result for device creation
	struct BuildResult
	{
		vk::Device device;
		std::vector<std::pair<std::uint32_t, std::uint32_t>> transferIndices;
		std::vector<std::pair<std::uint32_t, std::uint32_t>> computeIndices;
		std::vector<std::pair<std::uint32_t, std::uint32_t>> graphicsIndices;
	};

	static BuildResult BuildDevice(
		const vk::PhysicalDevice& physicalDevice,
		std::span<std::string> validationLayers,
		std::span<std::string> requiredExtensions);

	// Private helper constructor
	VulkanDevice(
		SchedulerType&,
		const vk::Instance&,
		const vk::PhysicalDevice&,
		BuildResult&&,
		std::uint32_t vulkanApiVersion);

public:
	VulkanDevice(
		SchedulerType&,
		const vk::Instance&,
		const vk::PhysicalDevice&,
		std::span<std::string>,
		std::span<std::string>,
		std::uint32_t vulkanApiVersion);

	~VulkanDevice()
	{
		if (device_)
		{
			device_.destroy();
		}
	}

	VulkanDevice(const VulkanDevice&) = delete;

	VulkanDevice(VulkanDevice&& other) noexcept :
		scheduler_(other.scheduler_),
		physicalDevice_(other.physicalDevice_),
		device_(other.device_),
		graphicsQueues_(std::move(other.graphicsQueues_)),
		computeQueues_(std::move(other.computeQueues_)),
		transferQueues_(std::move(other.transferQueues_)),
		allocator_(std::move(other.allocator_))
	{
		other.device_ = nullptr;  // Prevent source from destroying the device
	}

	VulkanDevice& operator=(const VulkanDevice&) = delete;

	VulkanDevice& operator=(VulkanDevice&& other) noexcept
	{
		if (this != &other)
		{
			if (device_)
			{
				device_.destroy();
			}
			scheduler_		= other.scheduler_;
			physicalDevice_ = other.physicalDevice_;
			device_			= other.device_;
			graphicsQueues_ = std::move(other.graphicsQueues_);
			computeQueues_	= std::move(other.computeQueues_);
			transferQueues_ = std::move(other.transferQueues_);
			allocator_		= std::move(other.allocator_);
			other.device_	= nullptr;
		}
		return *this;
	}

	void Synchronize()
	{
		device_.waitIdle();
	}

	const vk::Device& Logical() const
	{
		return device_;
	}

	const vk::PhysicalDevice& Physical() const
	{
		return physicalDevice_;
	}

	VulkanAllocator& Allocator() noexcept
	{
		return allocator_;
	}

	const VulkanAllocator& Allocator() const noexcept
	{
		return allocator_;
	}

	bool SurfaceSupported(vk::SurfaceKHR&);
	VulkanResult<std::uint32_t> HighFamilyIndex() const;

	std::span<VulkanQueue> GraphicsQueues()
	{
		return {graphicsQueues_};
	}

	std::span<VulkanQueue> ComputeQueues()
	{
		return {computeQueues_};
	}

	std::span<VulkanQueue> TransferQueues()
	{
		return {transferQueues_};
	}

private:
	SchedulerType& scheduler_;

	vk::PhysicalDevice physicalDevice_;
	vk::Device device_;

	std::vector<VulkanQueue> graphicsQueues_;
	std::vector<VulkanQueue> computeQueues_;
	std::vector<VulkanQueue> transferQueues_;

	VulkanAllocator allocator_;
};

// Public constructor - delegates to private constructor with BuildResult
template<SchedulerBackend SchedulerType>
VulkanDevice<SchedulerType>::VulkanDevice(
	SchedulerType& scheduler,
	const vk::Instance& instance,
	const vk::PhysicalDevice& physicalDevice,
	std::span<std::string> validationLayers,
	std::span<std::string> requiredExtensions,
	std::uint32_t vulkanApiVersion) :
	VulkanDevice(
		scheduler,
		instance,
		physicalDevice,
		BuildDevice(physicalDevice, validationLayers, requiredExtensions),
		vulkanApiVersion)
{
}

// Private helper constructor - properly initializes all members
template<SchedulerBackend SchedulerType>
VulkanDevice<SchedulerType>::VulkanDevice(
	SchedulerType& scheduler,
	const vk::Instance& instance,
	const vk::PhysicalDevice& physicalDevice,
	BuildResult&& buildResult,
	std::uint32_t vulkanApiVersion) :
	scheduler_(scheduler),
	physicalDevice_(physicalDevice),
	device_(buildResult.device),
	allocator_(instance, physicalDevice_, device_, vulkanApiVersion)
{
	// Create queue objects from indices
	for (auto& indices: buildResult.transferIndices)
	{
		transferQueues_.emplace_back(device_, indices.first, indices.second);
	}

	for (auto& indices: buildResult.computeIndices)
	{
		computeQueues_.emplace_back(device_, indices.first, indices.second);
	}

	for (auto& indices: buildResult.graphicsIndices)
	{
		graphicsQueues_.emplace_back(device_, indices.first, indices.second);
	}
}

template<SchedulerBackend SchedulerType>
typename VulkanDevice<SchedulerType>::BuildResult VulkanDevice<SchedulerType>::BuildDevice(
	const vk::PhysicalDevice& physicalDevice,
	std::span<std::string> validationLayers,
	std::span<std::string> requiredExtensions)
{
	// Convert strings to c-strings
	std::vector<const char*> cValidationLayers;
	cValidationLayers.reserve(validationLayers.size());
	for (auto& layer: validationLayers)
	{
		cValidationLayers.push_back(layer.c_str());
	}

	std::vector<const char*> cExtensions;
	cExtensions.reserve(requiredExtensions.size());
	for (auto& extension: requiredExtensions)
	{
		cExtensions.push_back(extension.c_str());
	}

	// TODO: Validate validation layers
	if constexpr (Compiler::Debug())
	{
		std::vector<vk::LayerProperties> availableLayers = physicalDevice.enumerateDeviceLayerProperties();
	}

	// TODO: Validate extensions
	std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

	// TODO: Query properties
	vk::PhysicalDeviceProperties deviceProperties		= physicalDevice.getProperties();
	vk::PhysicalDeviceFeatures deviceFeatures			= physicalDevice.getFeatures();
	vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

	// Start queue selection
	std::vector<vk::QueueFamilyProperties2> queueFamilyProperties = physicalDevice.getQueueFamilyProperties2();

	std::vector<std::pair<std::uint32_t, std::uint32_t>> transferIndices;
	std::vector<std::pair<std::uint32_t, std::uint32_t>> computeIndices;
	std::vector<std::pair<std::uint32_t, std::uint32_t>> graphicsIndices;

	std::vector<std::uint32_t> familyQueuesUsed(queueFamilyProperties.size(), 0);

	std::uint32_t maxThreads = std::thread::hardware_concurrency();

	// Iterate over all the queue families prioritizing dedicated hardware
	for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(queueFamilyProperties.size()); ++i)
	{
		auto& queueFamily = queueFamilyProperties[i].queueFamilyProperties;

		std::uint32_t count	 = (std::min)(queueFamily.queueCount, maxThreads);
		familyQueuesUsed[i] += count;

		// Transfer
		if ((queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) &&
			!(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
			!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
		{
			transferIndices.reserve(count);
			for (std::uint32_t t = 0; t < count; ++t)
			{
				transferIndices.emplace_back(i, t);
			}
		}

		// Compute
		else if (
			(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
			!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
		{
			computeIndices.reserve(count);
			for (std::uint32_t t = 0; t < count; ++t)
			{
				computeIndices.emplace_back(i, t);
			}
		}

		// Graphics
		else if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			graphicsIndices.reserve(count);
			for (std::uint32_t t = 0; t < count; ++t)
			{
				graphicsIndices.emplace_back(i, t);
			}
		}
	}

	// TODO: use remaining queues to supplement other queue types

	// Create the info structures
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::vector<std::vector<float>> priorities;
	queueCreateInfos.resize(queueFamilyProperties.size());
	priorities.resize(queueFamilyProperties.size());

	for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(queueFamilyProperties.size()); ++i)
	{
		// TODO: calculate priorities alongside Queue selection
		priorities[i].resize(familyQueuesUsed[i], 1.0f);

		vk::DeviceQueueCreateInfo deviceQueueCreateInfo;
		deviceQueueCreateInfo.flags			   = vk::DeviceQueueCreateFlags();
		deviceQueueCreateInfo.queueFamilyIndex = static_cast<std::uint32_t>(i);
		deviceQueueCreateInfo.queueCount	   = familyQueuesUsed[i];
		deviceQueueCreateInfo.pQueuePriorities = priorities[i].data();

		queueCreateInfos[i] = (deviceQueueCreateInfo);
	}

	// Create the device

	// Enable VK_EXT_extended_dynamic_state3 for dynamic blend state (No Graphics API pattern)
	vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Features;
	dynamicState3Features.extendedDynamicState3ColorBlendEnable = vk::True;
	dynamicState3Features.extendedDynamicState3ColorBlendEquation = vk::True;
	dynamicState3Features.extendedDynamicState3ColorWriteMask = vk::True;
	dynamicState3Features.extendedDynamicState3LogicOpEnable = vk::True;

	// Enable VK_EXT_descriptor_buffer features for bindless texture heap
	vk::PhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatures;
	descriptorBufferFeatures.pNext = &dynamicState3Features;
	descriptorBufferFeatures.descriptorBuffer = vk::True;
	descriptorBufferFeatures.descriptorBufferPushDescriptors = vk::True;

	// Enable Vulkan 1.4 features
	vk::PhysicalDeviceVulkan14Features vulkan14Features;
	vulkan14Features.pNext = &descriptorBufferFeatures;
	vulkan14Features.maintenance5 = vk::True;
	vulkan14Features.maintenance6 = vk::True;
	vulkan14Features.pushDescriptor = vk::True;

	// Enable Vulkan 1.3 features (synchronization2 is required for vkQueueSubmit2)
	vk::PhysicalDeviceVulkan13Features vulkan13Features;
	vulkan13Features.pNext = &vulkan14Features;
	vulkan13Features.synchronization2 = vk::True;
	vulkan13Features.dynamicRendering = vk::True;
	vulkan13Features.maintenance4 = vk::True;

	// Enable Vulkan 1.2 features
	vk::PhysicalDeviceVulkan12Features vulkan12Features;
	vulkan12Features.pNext = &vulkan13Features;
	vulkan12Features.timelineSemaphore = vk::True;
	vulkan12Features.bufferDeviceAddress = vk::True;
	vulkan12Features.descriptorIndexing = vk::True;
	vulkan12Features.runtimeDescriptorArray = vk::True;
	vulkan12Features.descriptorBindingPartiallyBound = vk::True;
	vulkan12Features.descriptorBindingVariableDescriptorCount = vk::True;
	vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = vk::True;  // For bindless texture/sampler heaps (covers SAMPLER, COMBINED_IMAGE_SAMPLER, SAMPLED_IMAGE)
	vulkan12Features.shaderSampledImageArrayNonUniformIndexing = vk::True;
	vulkan12Features.shaderStorageBufferArrayNonUniformIndexing = vk::True;  // For bindless buffer arrays
	vulkan12Features.scalarBlockLayout = vk::True;  // Required for PhysicalStorageBuffer with packed structs (vec3 at non-16-byte offsets)

	// Enable Vulkan 1.1 features
	vk::PhysicalDeviceVulkan11Features vulkan11Features;
	vulkan11Features.pNext = &vulkan12Features;
	vulkan11Features.shaderDrawParameters = vk::True;  // Required for SV_VertexID/SV_InstanceID in shaders

	// Base features
	vk::PhysicalDeviceFeatures2 deviceFeatures2;
	deviceFeatures2.pNext = &vulkan11Features;
	deviceFeatures2.features.shaderInt64 = vk::True;  // Required for uint64_t GPU pointers in shaders

	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.pNext					 = &deviceFeatures2;
	deviceCreateInfo.flags					 = vk::DeviceCreateFlags();
	deviceCreateInfo.queueCreateInfoCount	 = static_cast<std::uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos		 = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount	 = static_cast<std::uint32_t>(cExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = cExtensions.data();
	deviceCreateInfo.enabledLayerCount		 = static_cast<std::uint32_t>(cValidationLayers.size());
	deviceCreateInfo.ppEnabledLayerNames	 = cValidationLayers.data();

	vk::Device device = physicalDevice.createDevice(deviceCreateInfo);

	return BuildResult {
		.device			 = device,
		.transferIndices = std::move(transferIndices),
		.computeIndices	 = std::move(computeIndices),
		.graphicsIndices = std::move(graphicsIndices)};
}

template<SchedulerBackend SchedulerType>
bool VulkanDevice<SchedulerType>::SurfaceSupported(vk::SurfaceKHR& surface)
{
	bool supported = true;
	for (auto& graphicsQueue: graphicsQueues_)
	{
		if (!physicalDevice_.getSurfaceSupportKHR(graphicsQueue.FamilyIndex(), surface))
		{
			return false;
		}
	}

	return supported;
}

template<SchedulerBackend SchedulerType>
VulkanResult<std::uint32_t> VulkanDevice<SchedulerType>::HighFamilyIndex() const
{
	if (!graphicsQueues_.empty())
	{
		return graphicsQueues_[0].FamilyIndex();
	}

	if (!computeQueues_.empty())
	{
		return computeQueues_[0].FamilyIndex();
	}

	if (!transferQueues_.empty())
	{
		return transferQueues_[0].FamilyIndex();
	}

	return std::unexpected(VulkanError::DeviceNotFound);
}
