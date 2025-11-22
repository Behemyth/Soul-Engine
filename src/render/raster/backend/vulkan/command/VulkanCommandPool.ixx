export module synodic.soul.raster.backend.vulkan:command_pool;


import std;
import vulkan_hpp;
import :device;
import synodic.soul.scheduler;

export class VulkanCommandPool final {

public:

   VulkanCommandPool(std::shared_ptr<SchedulerModule>&, const VulkanDevice&);
	~VulkanCommandPool();

	VulkanCommandPool(const VulkanCommandPool&) = delete;
	VulkanCommandPool(VulkanCommandPool&&) noexcept = default;

	VulkanCommandPool& operator=(const VulkanCommandPool&) = delete;
	VulkanCommandPool& operator=(VulkanCommandPool&&) noexcept = default;

	const vk::CommandPool& Handle() const;


private:

	std::shared_ptr<SchedulerModule> scheduler_;
	vk::Device device_;

	// TODO: Replace with actual thread-local storage when available
	// ThreadLocal<vk::CommandPool> commandPool_;
	vk::CommandPool commandPool_;
};
