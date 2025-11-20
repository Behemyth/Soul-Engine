module;

#include <vulkan/vulkan.hpp>

export module render.raster.vulkan:pipeline;

import synodic.soul.engine;
import :render_pass;
import :shader;
import :pipeline_cache;
import :pipeline_layout;

import <vector>;

using std::uint32_t = std::uint32_t;

export class VulkanPipeline {

public:

	VulkanPipeline(const vk::Device&, nonstd::span<VulkanShader>,
		const vk::RenderPass&,
		std::uint32_t);
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
