module synodic.soul.raster.backend.vulkan;

import vulkan;
import std;
import :dispatch;

namespace
{

	// Debug callback for Vulkan validation layers using vulkan-hpp types
	vk::Bool32 DebugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		vk::DebugUtilsMessageTypeFlagsEXT messageType,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		// Determine severity string
		const char* severityStr = "UNKNOWN";
		if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		{
			severityStr = "ERROR";
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			severityStr = "WARNING";
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
		{
			severityStr = "INFO";
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
		{
			severityStr = "VERBOSE";
		}

		// Determine type string
		const char* typeStr = "";
		if (messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
		{
			typeStr = "VALIDATION";
		}
		else if (messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
		{
			typeStr = "PERFORMANCE";
		}
		else if (messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
		{
			typeStr = "GENERAL";
		}

		std::println(std::cerr, "[Vulkan {} {}] {}", severityStr, typeStr, pCallbackData->pMessage);

		// Return false to indicate the call should not be aborted
		return vk::False;
	}

}  // anonymous namespace

VulkanInstance::VulkanInstance(
	const vk::ApplicationInfo& appInfo,
	const std::span<std::string> validationLayers,
	const std::span<std::string> requiredExtensions)
{
	// Initialize dynamic dispatcher with global Vulkan functions BEFORE creating instance
	InitializeVulkanDispatcher();
	
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

	// TODO: Validate layers
	if constexpr (Compiler::Debug())
	{
		std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
	}

	// TODO: Validate extensions
	std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();

	vk::InstanceCreateInfo instanceCreationInfo;
	instanceCreationInfo.pApplicationInfo		 = &appInfo;
	instanceCreationInfo.enabledExtensionCount	 = static_cast<std::uint32_t>(cExtensions.size());
	instanceCreationInfo.ppEnabledExtensionNames = cExtensions.data();
	instanceCreationInfo.enabledLayerCount		 = static_cast<std::uint32_t>(cValidationLayers.size());
	instanceCreationInfo.ppEnabledLayerNames	 = cValidationLayers.data();
	
	// NOTE: GPU-Assisted Validation (GPU-AV) is incompatible with VK_EXT_descriptor_buffer.
	// GPU-AV disables all shader instrumentation when descriptor buffers are used.
	// For now, rely on Core Validation only. Debug printf still works for shader debugging.
	// Watch for future SDK releases that may add GPU-AV descriptor buffer support.
	// See: https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/docs/gpu_av_descriptor_buffer.md

	instance_ = createInstance(instanceCreationInfo);
	
	// Initialize instance-level functions in the dynamic dispatcher
	InitializeVulkanDispatcherInstance(instance_);

	if constexpr (Compiler::Debug())
	{
		vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
		messengerCreateInfo.flags = static_cast<vk::DebugUtilsMessengerCreateFlagBitsEXT>(0);
		// Only show warnings and errors (not verbose/info) for cleaner output
		messengerCreateInfo.messageSeverity =
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		messengerCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
										  vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
										  vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		messengerCreateInfo.pfnUserCallback = reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(DebugCallback);
		messengerCreateInfo.pUserData		= nullptr;

		// Load extension function dynamically
		auto createFunc = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			instance_.getProcAddr("vkCreateDebugUtilsMessengerEXT"));

		if (createFunc)
		{
			VkDebugUtilsMessengerEXT messenger;
			auto createInfo = static_cast<VkDebugUtilsMessengerCreateInfoEXT>(messengerCreateInfo);
			if (createFunc(instance_, &createInfo, nullptr, &messenger) == VK_SUCCESS)
			{
				debugMessenger_ = messenger;
			}
		}
	}
}

VulkanInstance::~VulkanInstance()
{
	if (!instance_) {
		return;
	}

	if constexpr (Compiler::Debug())
	{
		// Load extension function dynamically
		auto destroyFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			instance_.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));

		if (destroyFunc && debugMessenger_)
		{
			destroyFunc(instance_, debugMessenger_, nullptr);
		}
	}

	instance_.destroy();
}

const vk::Instance& VulkanInstance::Handle() const
{
	return instance_;
}

std::vector<VulkanPhysicalDevice> VulkanInstance::EnumeratePhysicalDevices()
{
	auto vkPhysicalDevices = instance_.enumeratePhysicalDevices();

	std::vector<VulkanPhysicalDevice> physicalDevices;

	for (auto& physicalDevice: vkPhysicalDevices)
	{
		physicalDevices.emplace_back(instance_, physicalDevice);
	}

	return physicalDevices;
}
