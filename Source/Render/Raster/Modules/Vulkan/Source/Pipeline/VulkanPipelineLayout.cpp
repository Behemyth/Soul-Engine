#include "VulkanPipelineLayout.h"

VulkanPipelineLayout::VulkanPipelineLayout(const vk::Device& device):
	device_(device)
{
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;


	pipelineLayout_ = device_.createPipelineLayout(pipelineLayoutInfo, nullptr);
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{

	device_.destroyPipelineLayout(pipelineLayout_);

}

const vk::PipelineLayout& VulkanPipelineLayout::Handle()
{

	return pipelineLayout_;

}