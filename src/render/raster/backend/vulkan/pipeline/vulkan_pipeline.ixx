export module synodic.soul.raster.backend.vulkan:pipeline;

import std;
import vulkan;

import :render_pass;
import :shader;
import :pipeline_cache;
import :pipeline_layout;

import std;

// Vertex format for pipeline creation
export enum class VertexFormat {
	None,       // No vertex input (shader generates vertices)
	PBR,        // PBRVertex format (position, normal, tangent, texcoord) - 48 bytes
	Custom,     // Use custom vertex attributes from reflection or explicit config
};

// Shader model determines pipeline type (vertex-based vs mesh shader)
export enum class ShaderModel {
	Vertex,     // Traditional vertex + fragment pipeline
	Mesh,       // Mesh shader + fragment pipeline (no vertex input)
	MeshTask,   // Task + Mesh + fragment pipeline (with GPU culling)
};

// Custom vertex attribute for reflection-based or explicit configuration
export struct VertexAttribute {
	std::uint32_t location = 0;
	std::uint32_t offset = 0;
	vk::Format format = vk::Format::eR32G32B32A32Sfloat;
};

// Pipeline configuration options
export struct VulkanPipelineConfig {
	ShaderModel shaderModel = ShaderModel::Vertex;  // Vertex or Mesh shader pipeline
	VertexFormat vertexFormat = VertexFormat::None;
	vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
	vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
	vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
	vk::FrontFace frontFace = vk::FrontFace::eClockwise;  // CW due to negative viewport height Y-flip
	bool depthTest = false;               // Enable depth testing
	bool depthWrite = false;              // Enable depth writing
	bool depthOnly = false;               // Depth-only pass (no color attachment)
	vk::CompareOp depthCompareOp = vk::CompareOp::eLessOrEqual;  // For standard depth
	
	// Pipeline layout configuration (push constants, descriptor sets)
	std::optional<PipelineLayoutConfig> layoutConfig;
	
	// Helper to enable PBR vertex format with transform push constants (no descriptor sets)
	static VulkanPipelineConfig PBRWithTransforms() {
		VulkanPipelineConfig config;
		config.vertexFormat = VertexFormat::PBR;
		config.depthTest = true;
		config.depthWrite = true;
		config.layoutConfig = PipelineLayoutConfig::WithTransformPushConstants();
		return config;
	}
	
	// Helper for PBR with transforms and material/lighting descriptor sets
	static VulkanPipelineConfig PBRWithDescriptors(vk::DescriptorSetLayout materialLayout) {
		VulkanPipelineConfig config;
		config.vertexFormat = VertexFormat::PBR;
		config.depthTest = true;
		config.depthWrite = true;
		auto layoutCfg = PipelineLayoutConfig::WithTransformPushConstants();
		layoutCfg.descriptorSetLayouts.push_back(materialLayout);
		config.layoutConfig = std::move(layoutCfg);
		return config;
	}
	
	// Helper for depth-only pre-pass with PBR vertices
	static VulkanPipelineConfig DepthPrePass() {
		VulkanPipelineConfig config;
		config.vertexFormat = VertexFormat::PBR;
		config.depthTest = true;
		config.depthWrite = true;
		config.depthOnly = true;
		config.depthCompareOp = vk::CompareOp::eLess;
		config.layoutConfig = PipelineLayoutConfig::WithTransformPushConstants();
		return config;
	}
	
	// Custom vertex layout (used when vertexFormat == Custom)
	std::vector<VertexAttribute> customVertexAttributes;
	std::uint32_t customVertexStride = 0;
	
	// Helper for custom vertex format from explicit attributes
	static VulkanPipelineConfig WithCustomVertexFormat(
		std::vector<VertexAttribute> attributes,
		std::uint32_t stride,
		vk::DescriptorSetLayout descriptorLayout = nullptr)
	{
		VulkanPipelineConfig config;
		config.vertexFormat = VertexFormat::Custom;
		config.customVertexAttributes = std::move(attributes);
		config.customVertexStride = stride;
		config.depthTest = true;
		config.depthWrite = true;
		auto layoutCfg = PipelineLayoutConfig::WithTransformPushConstants();
		if (descriptorLayout) {
			layoutCfg.descriptorSetLayouts.push_back(descriptorLayout);
		}
		config.layoutConfig = std::move(layoutCfg);
		return config;
	}
	
	// Helper for mesh shader pipeline with bindless data (no vertex input)
	static VulkanPipelineConfig MeshShaderBindless() {
		VulkanPipelineConfig config;
		config.shaderModel = ShaderModel::Mesh;
		config.vertexFormat = VertexFormat::None;  // Mesh shaders don't use vertex input
		config.depthTest = true;
		config.depthWrite = true;
		return config;
	}
	
	// Helper for task + mesh shader pipeline with GPU culling
	static VulkanPipelineConfig MeshTaskShaderBindless() {
		VulkanPipelineConfig config;
		config.shaderModel = ShaderModel::MeshTask;
		config.vertexFormat = VertexFormat::None;
		config.depthTest = true;
		config.depthWrite = true;
		return config;
	}
	
	// Helper for mesh shader depth-only pre-pass
	static VulkanPipelineConfig MeshShaderDepthPrePass() {
		VulkanPipelineConfig config;
		config.shaderModel = ShaderModel::Mesh;
		config.vertexFormat = VertexFormat::None;
		config.depthTest = true;
		config.depthWrite = true;
		config.depthOnly = true;
		config.depthCompareOp = vk::CompareOp::eLess;
		return config;
	}
};

export class VulkanPipeline {

public:

	// Original constructor with vertex input
	VulkanPipeline(const vk::Device&, std::span<VulkanShader>,
		const vk::RenderPass&,
		std::uint32_t);

	// New constructor with configuration
	VulkanPipeline(const vk::Device&, std::span<VulkanShader>,
		const vk::RenderPass&,
		std::uint32_t subPassIndex,
		const VulkanPipelineConfig& config);
	
	// Constructor with external pipeline layout (for bindless)
	VulkanPipeline(const vk::Device&, std::span<VulkanShader>,
		const vk::RenderPass&,
		std::uint32_t subPassIndex,
		const VulkanPipelineConfig& config,
		vk::PipelineLayout externalLayout);

	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline&) = delete;

	VulkanPipeline(VulkanPipeline&& other) noexcept :
		device_(other.device_),
		stages_(std::move(other.stages_)),
		pipelineCache_(std::move(other.pipelineCache_)),
		pipelineLayout_(std::move(other.pipelineLayout_)),
		externalLayout_(other.externalLayout_),
		pipeline_(other.pipeline_)
	{
		other.pipeline_ = nullptr;
		other.device_ = nullptr;
		other.externalLayout_ = nullptr;
	}

	VulkanPipeline& operator=(const VulkanPipeline&) = delete;

	VulkanPipeline& operator=(VulkanPipeline&& other) noexcept {
		if (this != &other) {
			if (pipeline_) {
				device_.destroyPipeline(pipeline_);
			}
			device_ = other.device_;
			stages_ = std::move(other.stages_);
			pipelineCache_ = std::move(other.pipelineCache_);
			pipelineLayout_ = std::move(other.pipelineLayout_);
			externalLayout_ = other.externalLayout_;
			pipeline_ = other.pipeline_;
			other.pipeline_ = nullptr;
			other.device_ = nullptr;
			other.externalLayout_ = nullptr;
		}
		return *this;
	}

	[[nodiscard]] const vk::Pipeline& Handle() const;
	[[nodiscard]] vk::PipelineLayout LayoutHandle() const { 
		return externalLayout_ ? externalLayout_ : pipelineLayout_.Handle(); 
	}
	[[nodiscard]] const VulkanPipelineLayout& Layout() const { return pipelineLayout_; }


private:

	void CreatePipeline(std::span<VulkanShader> shaders,
		const vk::RenderPass& renderPass,
		std::uint32_t subPassIndex,
		const VulkanPipelineConfig& config,
		vk::PipelineLayout layoutOverride = nullptr);

	vk::Device device_;

	std::vector<VulkanShader> stages_;

	VulkanPipelineCache pipelineCache_;
	VulkanPipelineLayout pipelineLayout_;
	vk::PipelineLayout externalLayout_ = nullptr;  // Non-owning, for bindless

	vk::Pipeline pipeline_;


};
