module synodic.soul.raster.backend.vulkan;

// Default constructor - empty layout (legacy compatibility)
VulkanPipelineLayout::VulkanPipelineLayout(const vk::Device& device):
	device_(device)
{
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	pipelineLayout_ = device_.createPipelineLayout(pipelineLayoutInfo, nullptr);
}

// Configured constructor - with push constants and descriptor sets
VulkanPipelineLayout::VulkanPipelineLayout(const vk::Device& device, const PipelineLayoutConfig& config):
	device_(device)
{
	// Convert our push constant ranges to Vulkan format
	pushConstantRanges_.reserve(config.pushConstantRanges.size());
	for (const auto& range : config.pushConstantRanges) {
		vk::PushConstantRange vkRange;
		vkRange.stageFlags = range.stageFlags;
		vkRange.offset = range.offset;
		vkRange.size = range.size;
		pushConstantRanges_.push_back(vkRange);
	}

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	
	// Descriptor set layouts
	pipelineLayoutInfo.setLayoutCount = static_cast<std::uint32_t>(config.descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = config.descriptorSetLayouts.empty() ? nullptr : config.descriptorSetLayouts.data();
	
	// Push constant ranges
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<std::uint32_t>(pushConstantRanges_.size());
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges_.empty() ? nullptr : pushConstantRanges_.data();

	pipelineLayout_ = device_.createPipelineLayout(pipelineLayoutInfo, nullptr);
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
	if (pipelineLayout_) {
		device_.destroyPipelineLayout(pipelineLayout_);
	}
}
