export module synodic.soul.raster.backend.vulkan:command_pool;


import std;
import vulkan_hpp;
import :device;
import synodic.soul.scheduler;

export template<typename SchedulerType> requires SchedulerBackend<SchedulerType>
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
		scheduler_.ForEachThread(TaskPriority::UX, [&]() noexcept {
			device_.destroyCommandPool(commandPool_);
		});
	}

	VulkanCommandPool(const VulkanCommandPool&) = delete;
	VulkanCommandPool(VulkanCommandPool&&) noexcept = default;

	VulkanCommandPool& operator=(const VulkanCommandPool&) = delete;
	VulkanCommandPool& operator=(VulkanCommandPool&&) noexcept = default;

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
