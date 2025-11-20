module;

#include <vulkan/vulkan.hpp>

export module render.raster.vulkan:pipeline_cache;

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
