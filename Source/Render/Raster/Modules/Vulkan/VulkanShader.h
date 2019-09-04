#pragma once

#include <vulkan/vulkan.hpp>

class Resource;
class VulkanDevice;

class VulkanShader {

public:

	VulkanShader(const vk::Device&, const vk::ShaderStageFlagBits&, const Resource&);
	~VulkanShader();

	VulkanShader(const VulkanShader&) = delete;
	VulkanShader(VulkanShader&& o) noexcept = default;

	VulkanShader& operator=(const VulkanShader&) = delete;
	VulkanShader& operator=(VulkanShader&& other) noexcept = default;

	[[nodiscard]] const vk::PipelineShaderStageCreateInfo& PipelineInfo() const;


private:

	vk::ShaderModule module_;
	vk::PipelineShaderStageCreateInfo info_;

	vk::Device device_;

};
