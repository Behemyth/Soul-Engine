#pragma once

#include "Render/Raster/Modules/Vulkan/VulkanRenderPass.h"
#include "Render/Raster/Modules/Vulkan/VulkanShader.h"
#include "VulkanPipelineCache.h"
#include "VulkanPipelineLayout.h"

#include <vulkan/vulkan.hpp>

class Resource;
class VulkanDevice;
class Vertex;

class VulkanPipeline {

public:

	VulkanPipeline(const vk::Device&, nonstd::span<VulkanShader>,
		const vk::RenderPass&,
		uint);
	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline&) = delete;
	VulkanPipeline(VulkanPipeline&&) noexcept = default;

	VulkanPipeline& operator=(const VulkanPipeline&) = delete;
	VulkanPipeline& operator=(VulkanPipeline&&) noexcept = default;

	[[nodiscard]] const vk::Pipeline& Handle() const;


private:

	vk::Device device_;

	std::vector<VulkanShader> stages_;

	VulkanPipelineCache pipelineCache_;
	VulkanPipelineLayout pipelineLayout_;

	vk::Pipeline pipeline_;


};
