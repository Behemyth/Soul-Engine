#pragma once

#include <vulkan/vulkan.hpp>


class VulkanPipelineLayout {

public:

	VulkanPipelineLayout(const vk::Device& device);
	~VulkanPipelineLayout();

	VulkanPipelineLayout(const VulkanPipelineLayout&) = delete;
	VulkanPipelineLayout(VulkanPipelineLayout&&) noexcept = default;

	VulkanPipelineLayout& operator=(const VulkanPipelineLayout&) = delete;
	VulkanPipelineLayout& operator=(VulkanPipelineLayout&&) noexcept = default;

	const vk::PipelineLayout& Handle();

private:

	vk::Device device_;
	vk::PipelineLayout pipelineLayout_;

};
