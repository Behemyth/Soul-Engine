#include "VulkanPipeline.h"

#include "Core/Geometry/Vertex.h"

VulkanPipeline::VulkanPipeline(const vk::Device& device,
	const nonstd::span<VulkanShader> shaders,
	const vk::RenderPass& renderPass,
	const uint subPassIndex):
	device_(device), 
	pipelineCache_(device_),
	pipelineLayout_(device_)
{

	// TODO: Refactor and move vertex attribute and bindings.
	vk::VertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = vk::VertexInputRate::eVertex;

	std::array<vk::VertexInputAttributeDescription, 1> attributeDescriptions;

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = offsetof(Vertex, position);  // TODO: C++23 Reflection

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	vk::PipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eClockwise;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	vk::PipelineMultisampleStateCreateInfo multiSampling;
	multiSampling.sampleShadingEnable = VK_FALSE;
	multiSampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

	vk::PipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.colorWriteMask =
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = VK_FALSE;

	vk::PipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.logicOpEnable = VK_FALSE;
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
	depthStencil.depthTestEnable = VK_TRUE; 
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = vk::CompareOp::eLessOrEqual;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.back.failOp = vk::StencilOp::eKeep;
	depthStencil.back.passOp = vk::StencilOp::eKeep;
	depthStencil.back.compareOp = vk::CompareOp::eAlways;
	depthStencil.stencilTestEnable = VK_FALSE;
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
	
	pipeline_ = device_.createGraphicsPipeline(pipelineCache_.Handle(), pipelineInfo);
}

VulkanPipeline::~VulkanPipeline()
{

	device_.destroyPipeline(pipeline_);

}

const vk::Pipeline& VulkanPipeline::Handle() const
{
	return pipeline_;
}