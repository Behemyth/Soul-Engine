#pragma once

#include "Types.h"
#include "Core/Utility/Thread/ThreadLocal.h"

#include <memory>
#include <vulkan/vulkan.hpp>

class SchedulerModule;
class VulkanDevice;

class VulkanCommandPool final {

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
