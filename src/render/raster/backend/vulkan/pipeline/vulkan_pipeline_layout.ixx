export module synodic.soul.raster.backend.vulkan:pipeline_layout;

import std;
import vulkan_hpp;

// Push constant range configuration
export struct PushConstantRange {
	vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
	std::uint32_t offset = 0;
	std::uint32_t size = 0;
};

// Pipeline layout configuration
export struct PipelineLayoutConfig {
	std::vector<PushConstantRange> pushConstantRanges;
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	
	// Helper for common 128-byte MVP + Model push constant (vertex + fragment)
	static PipelineLayoutConfig WithTransformPushConstants() {
		PipelineLayoutConfig config;
		config.pushConstantRanges.push_back({
			vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			0,
			128  // PushConstantData: MVP (64) + Model (64)
		});
		return config;
	}
	
	// Helper for bindless root constants (two 64-bit GPU pointers)
	// Matches RootConstants in shaders: vertexDataPtr + pixelDataPtr = 16 bytes
	static PipelineLayoutConfig WithBindlessRootConstants() {
		PipelineLayoutConfig config;
		config.pushConstantRanges.push_back({
			vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			0,
			16  // RootConstants: vertexDataPtr (8) + pixelDataPtr (8)
		});
		return config;
	}
	
	// Helper for vertex-only push constants
	static PipelineLayoutConfig WithVertexPushConstants(std::uint32_t size) {
		PipelineLayoutConfig config;
		config.pushConstantRanges.push_back({
			vk::ShaderStageFlagBits::eVertex,
			0,
			size
		});
		return config;
	}
};

export class VulkanPipelineLayout {

public:

	// Default constructor - empty layout (legacy compatibility)
	VulkanPipelineLayout(const vk::Device& device);
	
	// Configured constructor - with push constants and descriptor sets
	VulkanPipelineLayout(const vk::Device& device, const PipelineLayoutConfig& config);
	
	~VulkanPipelineLayout();

	VulkanPipelineLayout(const VulkanPipelineLayout&) = delete;

	VulkanPipelineLayout(VulkanPipelineLayout&& other) noexcept :
		device_(other.device_),
		pipelineLayout_(other.pipelineLayout_),
		pushConstantRanges_(std::move(other.pushConstantRanges_))
	{
		other.pipelineLayout_ = nullptr;
		other.device_ = nullptr;
	}

	VulkanPipelineLayout& operator=(const VulkanPipelineLayout&) = delete;

	VulkanPipelineLayout& operator=(VulkanPipelineLayout&& other) noexcept {
		if (this != &other) {
			if (pipelineLayout_) {
				device_.destroyPipelineLayout(pipelineLayout_);
			}
			device_ = other.device_;
			pipelineLayout_ = other.pipelineLayout_;
			pushConstantRanges_ = std::move(other.pushConstantRanges_);
			other.pipelineLayout_ = nullptr;
			other.device_ = nullptr;
		}
		return *this;
	}

	[[nodiscard]] const vk::PipelineLayout& Handle() const { return pipelineLayout_; }
	[[nodiscard]] vk::PipelineLayout& Handle() { return pipelineLayout_; }
	
	// Get push constant ranges for validation
	[[nodiscard]] const std::vector<vk::PushConstantRange>& PushConstantRanges() const { return pushConstantRanges_; }
	
	// Check if this layout has push constants
	[[nodiscard]] bool HasPushConstants() const { return !pushConstantRanges_.empty(); }
	
	// Get total push constant size
	[[nodiscard]] std::uint32_t TotalPushConstantSize() const {
		std::uint32_t total = 0;
		for (const auto& range : pushConstantRanges_) {
			total = std::max(total, range.offset + range.size);
		}
		return total;
	}

private:

	vk::Device device_;
	vk::PipelineLayout pipelineLayout_;
	std::vector<vk::PushConstantRange> pushConstantRanges_;

};
