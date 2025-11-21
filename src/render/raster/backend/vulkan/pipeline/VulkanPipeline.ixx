export module render.raster.vulkan:pipeline;

import std;
import vulkan_hpp;

import :render_pass;
import :shader;
import :pipeline_cache;
import :pipeline_layout;

import std;

export class VulkanPipeline {

public:

	VulkanPipeline(const vk::Device&, std::span<VulkanShader>,
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
