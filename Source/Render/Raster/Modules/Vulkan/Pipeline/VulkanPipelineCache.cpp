#include "VulkanPipelineCache.h"

VulkanPipelineCache::VulkanPipelineCache(const vk::Device& device):
	device_(device)
{

	vk::PipelineCacheCreateInfo pipelineCreateInfo;
	// TODO: pipeline serialization n' such

	pipelineCache_ = device_.createPipelineCache(pipelineCreateInfo);

}

VulkanPipelineCache::~VulkanPipelineCache()
{

	device_.destroyPipelineCache(pipelineCache_);

}

const vk::PipelineCache& VulkanPipelineCache::Handle()
{

	return pipelineCache_;

}