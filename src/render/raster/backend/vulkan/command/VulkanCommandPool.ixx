export module synodic.soul.raster.backend.vulkan:command_pool;


import std;
import vulkan_hpp;
import :device;
import synodic.soul.scheduler;

export template<SchedulerBackend SchedulerType>
class VulkanCommandPool final {

public:

   VulkanCommandPool(SchedulerType& scheduler, const VulkanDevice<SchedulerType>& device) :
		scheduler_(scheduler),
		device_(device.Logical())
	{
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient |
						 vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		auto familyIndexResult = device.HighFamilyIndex();
		if (!familyIndexResult) {
			throw std::runtime_error("Failed to get queue family index");
		}
		poolInfo.queueFamilyIndex = familyIndexResult.value();

		scheduler_.ForEachThread(TaskPriority::UX, [&]() {
			commandPool_ = device_.createCommandPool(poolInfo);
		});
	}

	~VulkanCommandPool() {
		if (commandPool_) {
			scheduler_.ForEachThread(TaskPriority::UX, [&]() noexcept {
				device_.destroyCommandPool(commandPool_);
			});
		}
	}

	VulkanCommandPool(const VulkanCommandPool&) = delete;

	VulkanCommandPool(VulkanCommandPool&& other) noexcept :
		scheduler_(other.scheduler_),
		device_(other.device_),
		commandPool_(other.commandPool_)
	{
		other.commandPool_ = nullptr;
		other.device_ = nullptr;
	}

	VulkanCommandPool& operator=(const VulkanCommandPool&) = delete;

	VulkanCommandPool& operator=(VulkanCommandPool&& other) noexcept {
		if (this != &other) {
			if (commandPool_) {
				scheduler_.ForEachThread(TaskPriority::UX, [&]() noexcept {
					device_.destroyCommandPool(commandPool_);
				});
			}
			device_ = other.device_;
			commandPool_ = other.commandPool_;
			other.commandPool_ = nullptr;
			other.device_ = nullptr;
		}
		return *this;
	}

	const vk::CommandPool& Handle() const {
		return commandPool_;
	}


private:

	SchedulerType& scheduler_;
	vk::Device device_;

	// TODO: Replace with actual thread-local storage when available
	// ThreadLocal<vk::CommandPool> commandPool_;
	vk::CommandPool commandPool_;
};
