module synodic.soul.raster.backend.vulkan;

// Vertex type - hardcoded for now until C++23 reflection
// TODO: Replace with proper reflection-based vertex description
struct VertexLayout {
	struct { float x, y, z; } position;
	struct { float x, y, z; } normal;
	struct { float x, y; } textureCoord;
	struct { float x, y, z; } velocity;
	std::uint32_t object;
};

VulkanPipeline::VulkanPipeline(const vk::Device& device,
	const std::span<VulkanShader> shaders,
	const vk::RenderPass& renderPass,
	const std::uint32_t subPassIndex):
	device_(device),
	pipelineCache_(device_),
	pipelineLayout_(device_)
{

	// TODO: Refactor and move vertex attribute and bindings.
	vk::VertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(VertexLayout);
	bindingDescription.inputRate = vk::VertexInputRate::eVertex;

	std::array<vk::VertexInputAttributeDescription, 1> attributeDescriptions;

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	// TODO: C++23 Reflection - offsetof not properly available in modules yet
	attributeDescriptions[0].offset = 0;  // position is first member

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssembly.primitiveRestartEnable = vk::False;

	vk::PipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eClockwise;
	rasterizer.depthBiasEnable = vk::False;
	rasterizer.depthClampEnable = vk::False;
	rasterizer.rasterizerDiscardEnable = vk::False;

	vk::PipelineMultisampleStateCreateInfo multiSampling;
	multiSampling.sampleShadingEnable = vk::False;
	multiSampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

	vk::PipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.colorWriteMask =
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = vk::False;

	vk::PipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.logicOpEnable = vk::False;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport
	};

	vk::PipelineDynamicStateCreateInfo dynamicState;
	dynamicState.flags = vk::PipelineDynamicStateCreateFlags();
	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();

	vk::PipelineDepthStencilStateCreateInfo depthStencil;
	depthStencil.depthTestEnable = vk::True;
	depthStencil.depthWriteEnable = vk::True;
	depthStencil.depthCompareOp = vk::CompareOp::eLessOrEqual;
	depthStencil.depthBoundsTestEnable = vk::False;
	depthStencil.back.failOp = vk::StencilOp::eKeep;
	depthStencil.back.passOp = vk::StencilOp::eKeep;
	depthStencil.back.compareOp = vk::CompareOp::eAlways;
	depthStencil.stencilTestEnable = vk::False;
	depthStencil.front = depthStencil.back;


	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(shaders.size());

	for (auto i = 0; i < shaderStages.size(); ++i) {

		shaderStages[i] = shaders[i].PipelineInfo();

	}


	vk::GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.flags = vk::PipelineCreateFlags();
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pTessellationState = nullptr;
	pipelineInfo.pViewportState = nullptr;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multiSampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout_.Handle();
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = subPassIndex;
	pipelineInfo.basePipelineHandle = nullptr;
	pipelineInfo.basePipelineIndex = 0;

	pipeline_ = device_.createGraphicsPipeline(pipelineCache_.Handle(), pipelineInfo).value;
}

VulkanPipeline::~VulkanPipeline()
{

	device_.destroyPipeline(pipeline_);

}

const vk::Pipeline& VulkanPipeline::Handle() const
{
	return pipeline_;
}
