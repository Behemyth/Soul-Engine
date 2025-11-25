module synodic.soul.raster.backend.vulkan;

// Legacy vertex type - kept for backwards compatibility
// TODO: Remove when all code uses PBRVertex
struct LegacyVertexLayout {
	struct { float x, y, z; } position;
	struct { float x, y, z; } normal;
	struct { float x, y; } textureCoord;
	struct { float x, y, z; } velocity;
	std::uint32_t object;
};

// PBR vertex layout - matches PBRVertex in synodic.soul.render.mesh:vertex
// position (12) + normal (12) + tangent (16) + texcoord (8) = 48 bytes
struct PBRVertexLayout {
	struct { float x, y, z; } position;      // location 0
	struct { float x, y, z; } normal;        // location 1
	struct { float x, y, z, w; } tangent;    // location 2
	struct { float x, y; } texCoord;         // location 3
};

static_assert(sizeof(PBRVertexLayout) == 48, "PBRVertexLayout must be 48 bytes");

VulkanPipeline::VulkanPipeline(const vk::Device& device,
	const std::span<VulkanShader> shaders,
	const vk::RenderPass& renderPass,
	const std::uint32_t subPassIndex):
	device_(device),
	pipelineCache_(device_),
	pipelineLayout_(device_)  // Empty layout for legacy
{
	// Default config with legacy vertex input enabled
	VulkanPipelineConfig config;
	config.vertexFormat = VertexFormat::Legacy;
	config.useVertexInput = true;
	config.depthTest = true;
	config.depthWrite = true;
	CreatePipeline(shaders, renderPass, subPassIndex, config);
}

VulkanPipeline::VulkanPipeline(const vk::Device& device,
	const std::span<VulkanShader> shaders,
	const vk::RenderPass& renderPass,
	const std::uint32_t subPassIndex,
	const VulkanPipelineConfig& config):
	device_(device),
	pipelineCache_(device_),
	pipelineLayout_(config.layoutConfig.has_value() 
		? VulkanPipelineLayout(device_, config.layoutConfig.value())
		: VulkanPipelineLayout(device_))
{
	CreatePipeline(shaders, renderPass, subPassIndex, config);
}

void VulkanPipeline::CreatePipeline(std::span<VulkanShader> shaders,
	const vk::RenderPass& renderPass,
	std::uint32_t subPassIndex,
	const VulkanPipelineConfig& config)
{
	// Vertex input state - based on vertex format
	vk::VertexInputBindingDescription bindingDescription;
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

	// Determine effective vertex format
	VertexFormat effectiveFormat = config.vertexFormat;
	if (effectiveFormat == VertexFormat::None && config.useVertexInput) {
		effectiveFormat = VertexFormat::Legacy;  // Backwards compatibility
	}

	if (effectiveFormat == VertexFormat::PBR) {
		// PBR vertex format: position, normal, tangent, texcoord
		// Layout: position(12) + normal(12) + tangent(16) + texcoord(8) = 48 bytes
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(PBRVertexLayout);  // 48 bytes
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;

		attributeDescriptions.resize(4);

		// Position - location 0, offset 0
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[0].offset = 0;

		// Normal - location 1, offset 12
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[1].offset = 12;

		// Tangent - location 2, offset 24 (vec4 for handedness)
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = vk::Format::eR32G32B32A32Sfloat;
		attributeDescriptions[2].offset = 24;

		// TexCoord - location 3, offset 40
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = vk::Format::eR32G32Sfloat;
		attributeDescriptions[3].offset = 40;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	} else if (effectiveFormat == VertexFormat::Legacy) {
		// Legacy vertex format: position only (for backwards compatibility)
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(LegacyVertexLayout);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;

		attributeDescriptions.resize(1);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[0].offset = 0;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	} else {
		// No vertex input - shader uses SV_VertexID with hardcoded vertices
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
	}

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.topology = config.topology;
	inputAssembly.primitiveRestartEnable = vk::False;

	vk::PipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.polygonMode = config.polygonMode;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = config.cullMode;
	rasterizer.frontFace = config.frontFace;
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

	// Dynamic state for viewport and scissor
	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo dynamicState;
	dynamicState.flags = vk::PipelineDynamicStateCreateFlags();
	dynamicState.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// Viewport state (dynamic, but need to specify count)
	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	vk::PipelineDepthStencilStateCreateInfo depthStencil;
	depthStencil.depthTestEnable = config.depthTest ? vk::True : vk::False;
	depthStencil.depthWriteEnable = config.depthWrite ? vk::True : vk::False;
	depthStencil.depthCompareOp = config.depthCompareOp;
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
	pipelineInfo.stageCount = static_cast<std::uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pTessellationState = nullptr;
	pipelineInfo.pViewportState = &viewportState;
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
	if (pipeline_) {
		device_.destroyPipeline(pipeline_);
	}
}

const vk::Pipeline& VulkanPipeline::Handle() const
{
	return pipeline_;
}
