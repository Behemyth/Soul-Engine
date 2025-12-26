module synodic.soul.raster.backend.vulkan;

// PBR vertex layout constants - matches synodic.soul.render.mesh:vertex
// position(vec3) + normal(vec3) + tangent(vec4) + texcoord(vec2) = 48 bytes
namespace PBRLayout {
	constexpr std::uint32_t Stride = 48;
	constexpr std::uint32_t PositionLocation = 0;
	constexpr std::uint32_t NormalLocation = 1;
	constexpr std::uint32_t TangentLocation = 2;
	constexpr std::uint32_t TexCoordLocation = 3;
	constexpr std::uint32_t PositionOffset = 0;
	constexpr std::uint32_t NormalOffset = 12;
	constexpr std::uint32_t TangentOffset = 24;
	constexpr std::uint32_t TexCoordOffset = 40;
}

VulkanPipeline::VulkanPipeline(const vk::Device& device,
	const std::span<VulkanShader> shaders,
	const vk::RenderPass& renderPass,
	const std::uint32_t subPassIndex):
	device_(device),
	pipelineCache_(device_),
	pipelineLayout_(device_)
{
	// Default config with PBR vertex input
	VulkanPipelineConfig config;
	config.vertexFormat = VertexFormat::PBR;
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

// Constructor with external pipeline layout (for bindless)
VulkanPipeline::VulkanPipeline(const vk::Device& device,
	const std::span<VulkanShader> shaders,
	const vk::RenderPass& renderPass,
	const std::uint32_t subPassIndex,
	const VulkanPipelineConfig& config,
	vk::PipelineLayout externalLayout):
	device_(device),
	pipelineCache_(device_),
	pipelineLayout_(device_),  // Empty internal layout (not used)
	externalLayout_(externalLayout)
{
	CreatePipeline(shaders, renderPass, subPassIndex, config, externalLayout);
}

void VulkanPipeline::CreatePipeline(std::span<VulkanShader> shaders,
	const vk::RenderPass& renderPass,
	std::uint32_t subPassIndex,
	const VulkanPipelineConfig& config,
	vk::PipelineLayout layoutOverride)
{
	// Vertex input state - based on vertex format
	vk::VertexInputBindingDescription bindingDescription;
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

	if (config.vertexFormat == VertexFormat::PBR) {
		// PBR vertex format: position, normal, tangent, texcoord
		bindingDescription.binding = 0;
		bindingDescription.stride = PBRLayout::Stride;
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;

		attributeDescriptions.resize(4);

		// Position - location 0
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = PBRLayout::PositionLocation;
		attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[0].offset = PBRLayout::PositionOffset;

		// Normal - location 1
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = PBRLayout::NormalLocation;
		attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[1].offset = PBRLayout::NormalOffset;

		// Tangent - location 2 (vec4 for handedness)
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = PBRLayout::TangentLocation;
		attributeDescriptions[2].format = vk::Format::eR32G32B32A32Sfloat;
		attributeDescriptions[2].offset = PBRLayout::TangentOffset;

		// TexCoord - location 3
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = PBRLayout::TexCoordLocation;
		attributeDescriptions[3].format = vk::Format::eR32G32Sfloat;
		attributeDescriptions[3].offset = PBRLayout::TexCoordOffset;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	} else if (config.vertexFormat == VertexFormat::Custom && !config.customVertexAttributes.empty()) {
		// Custom vertex format from reflection or explicit configuration
		bindingDescription.binding = 0;
		bindingDescription.stride = config.customVertexStride;
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;

		attributeDescriptions.reserve(config.customVertexAttributes.size());
		for (const auto& attr : config.customVertexAttributes) {
			vk::VertexInputAttributeDescription desc;
			desc.binding = 0;
			desc.location = attr.location;
			desc.format = attr.format;
			desc.offset = attr.offset;
			attributeDescriptions.push_back(desc);
		}

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

	// Use external layout if provided, otherwise use internal layout
	vk::PipelineLayout activeLayout = layoutOverride ? layoutOverride : pipelineLayout_.Handle();

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
	pipelineInfo.layout = activeLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = subPassIndex;
	pipelineInfo.basePipelineHandle = nullptr;
	pipelineInfo.basePipelineIndex = 0;

	// Validate critical state before pipeline creation
	if (!device_) {
		throw std::runtime_error("VulkanPipeline: Invalid device");
	}
	if (!activeLayout) {
		throw std::runtime_error("VulkanPipeline: Invalid pipeline layout");
	}
	if (!renderPass) {
		throw std::runtime_error("VulkanPipeline: Invalid render pass");
	}
	for (const auto& stage : shaderStages) {
		if (!stage.module) {
			throw std::runtime_error("VulkanPipeline: Invalid shader module");
		}
	}

	// Use try-catch to capture any exceptions from vulkan-hpp
	try {
		auto result = device_.createGraphicsPipeline(nullptr, pipelineInfo);  // Pass nullptr for cache to simplify
		if (result.result != vk::Result::eSuccess) {
			throw std::runtime_error("VulkanPipeline: Failed to create graphics pipeline - " +
				vk::to_string(result.result));
		}
		pipeline_ = result.value;
	} catch (const vk::SystemError& e) {
		throw std::runtime_error(std::string("VulkanPipeline: Vulkan error during pipeline creation - ") + e.what());
	} catch (const std::exception& e) {
		throw std::runtime_error(std::string("VulkanPipeline: Exception during pipeline creation - ") + e.what());
	}
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
