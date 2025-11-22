module synodic.soul.raster.backend.vulkan;

// Using static dispatcher - no dynamic loader needed with C++20 modules


VulkanInstance::VulkanInstance(const vk::ApplicationInfo& appInfo,
	std::span<std::string> validationLayers,
	std::span<std::string> requiredExtensions)
{

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

	// TODO: Validate layers
	if constexpr (Compiler::Debug()) {

		std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

	}

	// TODO: Validate extensions
	std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();

	vk::InstanceCreateInfo instanceCreationInfo;
	instanceCreationInfo.pApplicationInfo = &appInfo;
	instanceCreationInfo.enabledExtensionCount = static_cast<std::uint32_t>(cExtensions.size());
	instanceCreationInfo.ppEnabledExtensionNames = cExtensions.data();
	instanceCreationInfo.enabledLayerCount = static_cast<std::uint32_t>(cValidationLayers.size());
	instanceCreationInfo.ppEnabledLayerNames = cValidationLayers.data();

	instance_ = createInstance(instanceCreationInfo);

	// Note: Debug messenger requires dynamic dispatch or macro configuration
	// Static dispatcher doesn't support VK_EXT_debug_utils extension functions
	// For validation, check standard output or use a dynamic dispatcher
	if constexpr (false && Compiler::Debug()) {


		vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
		messengerCreateInfo.flags = vk::DebugUtilsMessengerCreateFlagBitsEXT(0);
		messengerCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
											  vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		messengerCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
										  vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
										  vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		// TODO: Fix DebugCallback with proper C API types for C++20 modules
		// messengerCreateInfo.pfnUserCallback = DebugCallback;
		messengerCreateInfo.pUserData = nullptr;

		debugMessenger_ =
			instance_.createDebugUtilsMessengerEXT(messengerCreateInfo);

	}

}

VulkanInstance::~VulkanInstance()
{

	if constexpr (false && Compiler::Debug()) {

		instance_.destroyDebugUtilsMessengerEXT(debugMessenger_);

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

	for (auto& physicalDevice : vkPhysicalDevices) {

		physicalDevices.emplace_back(instance_, physicalDevice);

	}

	return physicalDevices;

}

// TODO: Uncomment and fix when C API types are available in modules
// VkBool32 VulkanInstance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
// 	VkDebugUtilsMessageTypeFlagsEXT messageType,
// 	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
// 	void* pUserData)
// {
// 	throw NotImplemented();
// }
