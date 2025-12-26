

export module synodic.soul.raster.backend.vulkan:instance;
import std;
import vulkan;
import :physical_device;
import synodic.soul.engine;

export class VulkanInstance {

public:

	VulkanInstance(const vk::ApplicationInfo&,
		std::span<std::string>,
		std::span<std::string>);
	~VulkanInstance();

	VulkanInstance(const VulkanInstance&) = delete;

	VulkanInstance(VulkanInstance&& other) noexcept :
		instance_(other.instance_),
		debugMessenger_(other.debugMessenger_)
	{
		other.instance_ = nullptr;
		other.debugMessenger_ = nullptr;
	}

	VulkanInstance& operator=(const VulkanInstance&) = delete;

	VulkanInstance& operator=(VulkanInstance&& other) noexcept {
		if (this != &other) {
			// Destroy existing resources
			if (instance_) {
				if constexpr (Compiler::Debug()) {
					auto destroyFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
						instance_.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
					if (destroyFunc && debugMessenger_) {
						destroyFunc(instance_, debugMessenger_, nullptr);
					}
				}
				instance_.destroy();
			}
			instance_ = other.instance_;
			debugMessenger_ = other.debugMessenger_;
			other.instance_ = nullptr;
			other.debugMessenger_ = nullptr;
		}
		return *this;
	}

	const vk::Instance& Handle() const;

	std::vector<VulkanPhysicalDevice> EnumeratePhysicalDevices();

private:

	vk::Instance instance_;

	// Debug state
	vk::DebugUtilsMessengerEXT debugMessenger_;

};
