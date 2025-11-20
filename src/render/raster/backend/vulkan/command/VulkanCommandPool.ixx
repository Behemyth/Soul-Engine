module;

#include <vulkan/vulkan.hpp>

export module render.raster.vulkan:command_pool;

import synodic.soul.engine;
import :device;

import <memory>;

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

	ThreadLocal<vk::CommandPool> commandPool_;


};
