export module synodic.soul.raster.backend.vulkan:pipeline_cache;

import std;
import vulkan_hpp;

export class VulkanPipelineCache {

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
