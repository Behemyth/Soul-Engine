/**
 * @file vulkan_bindless_layout.ixx
 * @brief Global pipeline layout for bindless rendering
 * 
 * Creates a single shared pipeline layout for all bindless pipelines.
 * Uses descriptor indexing for texture/sampler heaps and push constants
 * for root GPU pointers.
 */
export module synodic.soul.raster.backend.vulkan:bindless_layout;

import std;
import vulkan_hpp;

import synodic.soul.raster;

/**
 * @brief Global bindless pipeline layout
 * 
 * Single layout used by all pipelines. Contains:
 * - Push constant for root data GPU pointer (8 bytes for vertex, 8 for pixel)
 * - Descriptor set 0 with unbounded texture and sampler arrays
 * 
 * Matches shader bindings:
 * - [[vk::binding(0, 0)]] Texture2D textureHeap[];
 * - [[vk::binding(1, 0)]] SamplerState samplerHeap[];
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
	 * @brief Get descriptor set layout for texture/sampler heap binding
	 */
	[[nodiscard]] vk::DescriptorSetLayout DescriptorSetLayout() const noexcept { 
		return descriptorSetLayout_; 
	}
	
	/**
	 * @brief Check if layout is valid
	 */
	[[nodiscard]] bool IsValid() const noexcept { return layout_ != nullptr; }

private:
	vk::Device device_ = nullptr;
	vk::PipelineLayout layout_ = nullptr;
	vk::DescriptorSetLayout descriptorSetLayout_ = nullptr;
};

// Implementation

VulkanBindlessLayout::VulkanBindlessLayout(vk::Device device)
	: device_(device)
{
	// Binding 0: Unbounded sampler array (fixed size, no variable count)
	vk::DescriptorSetLayoutBinding samplerBinding;
	samplerBinding.binding = 0;
	samplerBinding.descriptorType = vk::DescriptorType::eSampler;
	samplerBinding.descriptorCount = MaxSamplerCount;
	samplerBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
	samplerBinding.pImmutableSamplers = nullptr;
	
	// Binding 1: Unbounded texture array (sampled images) - variable count on last binding
	vk::DescriptorSetLayoutBinding textureBinding;
	textureBinding.binding = 1;
	textureBinding.descriptorType = vk::DescriptorType::eSampledImage;
	textureBinding.descriptorCount = MaxTextureCount;
	textureBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
	textureBinding.pImmutableSamplers = nullptr;
	
	std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { samplerBinding, textureBinding };
	
	// Enable descriptor indexing flags for each binding
	// VariableDescriptorCount can ONLY be on the last binding (highest binding number)
	std::array<vk::DescriptorBindingFlags, 2> bindingFlags = {
		// Binding 0 (samplers): no variable count
		vk::DescriptorBindingFlagBits::ePartiallyBound |
		vk::DescriptorBindingFlagBits::eUpdateAfterBind,
		
		// Binding 1 (textures): with variable count (must be last)
		vk::DescriptorBindingFlagBits::ePartiallyBound | 
		vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
		vk::DescriptorBindingFlagBits::eUpdateAfterBind
	};
	
	vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo;
	bindingFlagsInfo.bindingCount = static_cast<std::uint32_t>(bindingFlags.size());
	bindingFlagsInfo.pBindingFlags = bindingFlags.data();
	
	// Create descriptor set layout with update-after-bind for bindless
	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.pNext = &bindingFlagsInfo;
	layoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
	layoutInfo.bindingCount = static_cast<std::uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
	
	descriptorSetLayout_ = device_.createDescriptorSetLayout(layoutInfo);
	
	// Push constant range for root data pointers
	vk::PushConstantRange pushConstantRange;
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex | 
	                               vk::ShaderStageFlagBits::eFragment |
	                               vk::ShaderStageFlagBits::eCompute;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(RootConstants);
	
	// Pipeline layout with descriptor set and push constants
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	
	layout_ = device_.createPipelineLayout(pipelineLayoutInfo);
}

VulkanBindlessLayout::~VulkanBindlessLayout() {
	if (device_) {
		if (layout_) {
			device_.destroyPipelineLayout(layout_);
		}
		if (descriptorSetLayout_) {
			device_.destroyDescriptorSetLayout(descriptorSetLayout_);
		}
	}
}

VulkanBindlessLayout::VulkanBindlessLayout(VulkanBindlessLayout&& other) noexcept
	: device_(other.device_)
	, layout_(other.layout_)
	, descriptorSetLayout_(other.descriptorSetLayout_)
{
	other.device_ = nullptr;
	other.layout_ = nullptr;
	other.descriptorSetLayout_ = nullptr;
}

VulkanBindlessLayout& VulkanBindlessLayout::operator=(VulkanBindlessLayout&& other) noexcept {
	if (this != &other) {
		if (device_) {
			if (layout_) device_.destroyPipelineLayout(layout_);
			if (descriptorSetLayout_) device_.destroyDescriptorSetLayout(descriptorSetLayout_);
		}
		
		device_ = other.device_;
		layout_ = other.layout_;
		descriptorSetLayout_ = other.descriptorSetLayout_;
		
		other.device_ = nullptr;
		other.layout_ = nullptr;
		other.descriptorSetLayout_ = nullptr;
	}
	return *this;
}


