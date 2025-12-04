export module synodic.soul.raster.backend.vulkan:pipeline_cache;

import std;
import vulkan_hpp;

export class VulkanPipelineCache {

public:

	VulkanPipelineCache(const vk::Device& device);
	~VulkanPipelineCache();

	VulkanPipelineCache(const VulkanPipelineCache&) = delete;

	VulkanPipelineCache(VulkanPipelineCache&& other) noexcept :
		device_(other.device_),
		pipelineCache_(other.pipelineCache_)
	{
		other.pipelineCache_ = nullptr;
		other.device_ = nullptr;
	}

	VulkanPipelineCache& operator=(const VulkanPipelineCache&) = delete;

	VulkanPipelineCache& operator=(VulkanPipelineCache&& other) noexcept {
		if (this != &other) {
			if (pipelineCache_) {
				device_.destroyPipelineCache(pipelineCache_);
			}
			device_ = other.device_;
			pipelineCache_ = other.pipelineCache_;
			other.pipelineCache_ = nullptr;
			other.device_ = nullptr;
		}
		return *this;
	}

	const vk::PipelineCache& Handle();


private:

	vk::Device device_;
	vk::PipelineCache pipelineCache_;


};
