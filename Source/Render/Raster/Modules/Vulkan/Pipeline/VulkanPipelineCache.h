#pragma once

#include <vulkan/vulkan.hpp>


class VulkanPipelineCache {

public:

	VulkanPipelineCache(const vk::Device& device);
	~VulkanPipelineCache();

	VulkanPipelineCache(const VulkanPipelineCache&) = delete;
	VulkanPipelineCache(VulkanPipelineCache&&) noexcept = default;

	VulkanPipelineCache& operator=(const VulkanPipelineCache&) = delete;
	VulkanPipelineCache& operator=(VulkanPipelineCache&&) noexcept = default;

	const vk::PipelineCache& Handle();


private:

	vk::Device device_;
	vk::PipelineCache pipelineCache_;


};
