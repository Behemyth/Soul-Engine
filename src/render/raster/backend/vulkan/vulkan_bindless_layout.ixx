/**
 * @file vulkan_bindless_layout.ixx
 * @brief Global pipeline layout for bindless rendering
 * 
 * Creates a single shared pipeline layout for all bindless pipelines.
 * Uses VK_EXT_descriptor_buffer with two descriptor sets:
 * - Set 0: Sampler heap (sampler descriptor buffer)
 * - Set 1: Texture heap (resource descriptor buffer)
 * Push constants carry root GPU pointers for shader data.
 */
export module synodic.soul.raster.backend.vulkan:bindless_layout;

import std;
import vulkan;

import synodic.soul.raster;

/**
 * @brief Global bindless pipeline layout
 * 
 * Single layout used by all pipelines. Contains:
 * - Push constant for root data GPU pointer (8 bytes for vertex, 8 for pixel)
 * - Descriptor set 0: Sampler array (in sampler descriptor buffer)
 * - Descriptor set 1: Texture array (in resource descriptor buffer)
 * 
 * Matches shader bindings:
 * - [[vk::binding(0, 0)]] SamplerState samplerHeap[];  // Set 0, binding 0
 * - [[vk::binding(0, 1)]] Texture2D textureHeap[];     // Set 1, binding 0
 */
export class VulkanBindlessLayout {
public:
	/**
	 * @brief Root argument push constant layout
	 */
	struct RootConstants {
		GPUDeviceAddress vertexData;   // 8 bytes - GPU pointer to vertex shader data
		GPUDeviceAddress pixelData;    // 8 bytes - GPU pointer to pixel shader data
	};
	
	static_assert(sizeof(RootConstants) == 16, "Root constants must be 16 bytes");
	
	// Maximum number of textures/samplers in the heap
	// Set to a large number - actual usage is tracked via descriptorBindingPartiallyBound
	static constexpr std::uint32_t MaxTextureCount = 16384;
	static constexpr std::uint32_t MaxSamplerCount = 256;
	
	/**
	 * @brief Create the global bindless layout
	 */
	explicit VulkanBindlessLayout(vk::Device device);
	
	~VulkanBindlessLayout();
	
	VulkanBindlessLayout(const VulkanBindlessLayout&) = delete;
	VulkanBindlessLayout& operator=(const VulkanBindlessLayout&) = delete;
	
	VulkanBindlessLayout(VulkanBindlessLayout&& other) noexcept;
	VulkanBindlessLayout& operator=(VulkanBindlessLayout&& other) noexcept;
	
	/**
	 * @brief Get pipeline layout handle
	 */
	[[nodiscard]] vk::PipelineLayout Handle() const noexcept { return layout_; }
	
	/**
	 * @brief Get sampler descriptor set layout (set 0)
	 */
	[[nodiscard]] vk::DescriptorSetLayout SamplerSetLayout() const noexcept { 
		return samplerSetLayout_; 
	}
	
	/**
	 * @brief Get texture descriptor set layout (set 1)
	 */
	[[nodiscard]] vk::DescriptorSetLayout TextureSetLayout() const noexcept {
		return textureSetLayout_;
	}
	
	/**
	 * @brief Get descriptor set layout for legacy compatibility (returns sampler set)
	 * @deprecated Use SamplerSetLayout() or TextureSetLayout() instead
	 */
	[[nodiscard]] vk::DescriptorSetLayout DescriptorSetLayout() const noexcept { 
		return samplerSetLayout_; 
	}
	
	/**
	 * @brief Check if layout is valid
	 */
	[[nodiscard]] bool IsValid() const noexcept { return layout_ != nullptr; }

private:
	vk::Device device_ = nullptr;
	vk::PipelineLayout layout_ = nullptr;
	vk::DescriptorSetLayout samplerSetLayout_ = nullptr;  // Set 0: Samplers
	vk::DescriptorSetLayout textureSetLayout_ = nullptr;  // Set 1: Textures
};

// Implementation

VulkanBindlessLayout::VulkanBindlessLayout(vk::Device device)
	: device_(device)
{
	// Set 0, Binding 0: Sampler array (in sampler descriptor buffer)
	vk::DescriptorSetLayoutBinding samplerBinding;
	samplerBinding.binding = 0;
	samplerBinding.descriptorType = vk::DescriptorType::eSampler;
	samplerBinding.descriptorCount = MaxSamplerCount;
	samplerBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
	samplerBinding.pImmutableSamplers = nullptr;
	
	// Create sampler set layout (set 0) - for sampler descriptor buffer
	vk::DescriptorSetLayoutCreateInfo samplerLayoutInfo;
	samplerLayoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
	samplerLayoutInfo.bindingCount = 1;
	samplerLayoutInfo.pBindings = &samplerBinding;
	
	samplerSetLayout_ = device_.createDescriptorSetLayout(samplerLayoutInfo);
	
	// Set 1, Binding 0: Texture array (sampled images) (in resource descriptor buffer)
	vk::DescriptorSetLayoutBinding textureBinding;
	textureBinding.binding = 0;
	textureBinding.descriptorType = vk::DescriptorType::eSampledImage;
	textureBinding.descriptorCount = MaxTextureCount;
	textureBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
	textureBinding.pImmutableSamplers = nullptr;
	
	// Create texture set layout (set 1) - for resource descriptor buffer
	vk::DescriptorSetLayoutCreateInfo textureLayoutInfo;
	textureLayoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
	textureLayoutInfo.bindingCount = 1;
	textureLayoutInfo.pBindings = &textureBinding;
	
	textureSetLayout_ = device_.createDescriptorSetLayout(textureLayoutInfo);
	
	// Push constant range for root data pointers
	vk::PushConstantRange pushConstantRange;
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex | 
	                               vk::ShaderStageFlagBits::eFragment |
	                               vk::ShaderStageFlagBits::eCompute;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(RootConstants);
	
	// Pipeline layout with both descriptor sets (set 0 = samplers, set 1 = textures)
	std::array<vk::DescriptorSetLayout, 2> setLayouts = { samplerSetLayout_, textureSetLayout_ };
	
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setLayoutCount = static_cast<std::uint32_t>(setLayouts.size());
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	
	layout_ = device_.createPipelineLayout(pipelineLayoutInfo);
}

VulkanBindlessLayout::~VulkanBindlessLayout() {
	if (device_) {
		if (layout_) {
			device_.destroyPipelineLayout(layout_);
		}
		if (samplerSetLayout_) {
			device_.destroyDescriptorSetLayout(samplerSetLayout_);
		}
		if (textureSetLayout_) {
			device_.destroyDescriptorSetLayout(textureSetLayout_);
		}
	}
}

VulkanBindlessLayout::VulkanBindlessLayout(VulkanBindlessLayout&& other) noexcept
	: device_(other.device_)
	, layout_(other.layout_)
	, samplerSetLayout_(other.samplerSetLayout_)
	, textureSetLayout_(other.textureSetLayout_)
{
	other.device_ = nullptr;
	other.layout_ = nullptr;
	other.samplerSetLayout_ = nullptr;
	other.textureSetLayout_ = nullptr;
}

VulkanBindlessLayout& VulkanBindlessLayout::operator=(VulkanBindlessLayout&& other) noexcept {
	if (this != &other) {
		if (device_) {
			if (layout_) device_.destroyPipelineLayout(layout_);
			if (samplerSetLayout_) device_.destroyDescriptorSetLayout(samplerSetLayout_);
			if (textureSetLayout_) device_.destroyDescriptorSetLayout(textureSetLayout_);
		}
		
		device_ = other.device_;
		layout_ = other.layout_;
		samplerSetLayout_ = other.samplerSetLayout_;
		textureSetLayout_ = other.textureSetLayout_;
		
		other.device_ = nullptr;
		other.layout_ = nullptr;
		other.samplerSetLayout_ = nullptr;
		other.textureSetLayout_ = nullptr;
	}
	return *this;
}


