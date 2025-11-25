export module synodic.soul.raster.backend.vulkan:pipeline;

import std;
import vulkan_hpp;

import :render_pass;
import :shader;
import :pipeline_cache;
import :pipeline_layout;

import std;

// Vertex format for pipeline creation
export enum class VertexFormat {
	None,       // No vertex input (shader generates vertices)
	Legacy,     // Old VertexLayout format (position, normal, texcoord, velocity, object)
	PBR,        // PBRVertex format (position, normal, tangent, texcoord) - 48 bytes
};

// Pipeline configuration options
export struct VulkanPipelineConfig {
	VertexFormat vertexFormat = VertexFormat::None;
	bool useVertexInput = false;          // Deprecated: use vertexFormat instead
	vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
	vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
	vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
	vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;  // CCW for right-hand coords
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
		// Depth pre-pass only needs MVP (64 bytes), but we use full 128 for consistency
		config.layoutConfig = PipelineLayoutConfig::WithTransformPushConstants();
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

	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline&) = delete;

	VulkanPipeline(VulkanPipeline&& other) noexcept :
		device_(other.device_),
		stages_(std::move(other.stages_)),
		pipelineCache_(std::move(other.pipelineCache_)),
		pipelineLayout_(std::move(other.pipelineLayout_)),
		pipeline_(other.pipeline_)
	{
		other.pipeline_ = nullptr;
		other.device_ = nullptr;
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
			pipeline_ = other.pipeline_;
			other.pipeline_ = nullptr;
			other.device_ = nullptr;
		}
		return *this;
	}

	[[nodiscard]] const vk::Pipeline& Handle() const;
	[[nodiscard]] const VulkanPipelineLayout& Layout() const { return pipelineLayout_; }


private:

	void CreatePipeline(std::span<VulkanShader> shaders,
		const vk::RenderPass& renderPass,
		std::uint32_t subPassIndex,
		const VulkanPipelineConfig& config);

	vk::Device device_;

	std::vector<VulkanShader> stages_;

	VulkanPipelineCache pipelineCache_;
	VulkanPipelineLayout pipelineLayout_;

	vk::Pipeline pipeline_;


};
