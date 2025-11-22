module synodic.soul.raster.backend.vulkan;

VulkanCommandPool::VulkanCommandPool(std::shared_ptr<SchedulerModule>& scheduler,
	const VulkanDevice& device) :
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

	scheduler_->ForEachThread(TaskPriority::UX, [&]() {
		commandPool_ = device_.createCommandPool(poolInfo);
	});

}

VulkanCommandPool::~VulkanCommandPool()
{

	scheduler_->ForEachThread(TaskPriority::UX, [&]() noexcept {
		device_.destroyCommandPool(commandPool_);
	});

}


const vk::CommandPool& VulkanCommandPool::Handle() const
{

	return commandPool_;

}
