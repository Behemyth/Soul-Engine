/**
 * @file vulkan_bindless_layout.ixx
 * @brief Global pipeline layout for bindless rendering
 * 
 * Creates a single shared pipeline layout for all bindless pipelines.
 * No descriptor sets - all resource access via buffer device addresses
 * passed through a small push constant containing the root data pointer.
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
 * - No descriptor set layouts (fully bindless via buffer device address)
 * 
 * Shaders access resources via:
 * - Root data pointer passed as push constant
 * - Texture/sampler heap bound via vkCmdBindDescriptorBuffersEXT
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
	 * @brief Get descriptor set layout for texture heap binding
	 */
	[[nodiscard]] vk::DescriptorSetLayout TextureHeapLayout() const noexcept { 
		return textureHeapLayout_; 
	}
	
	/**
	 * @brief Check if layout is valid
	 */
	[[nodiscard]] bool IsValid() const noexcept { return layout_ != nullptr; }

private:
	vk::Device device_ = nullptr;
	vk::PipelineLayout layout_ = nullptr;
	vk::DescriptorSetLayout textureHeapLayout_ = nullptr;  // For descriptor buffer binding
};

// Implementation

VulkanBindlessLayout::VulkanBindlessLayout(vk::Device device)
	: device_(device)
{
	// Create descriptor set layout for descriptor buffer (texture + sampler heaps)
	// Using VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT
	vk::DescriptorSetLayoutCreateInfo heapLayoutInfo;
	heapLayoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
	heapLayoutInfo.bindingCount = 0;  // No bindings - we use buffer device addresses
	heapLayoutInfo.pBindings = nullptr;
	
	textureHeapLayout_ = device_.createDescriptorSetLayout(heapLayoutInfo);
	
	// Push constant range for root data pointers
	vk::PushConstantRange pushConstantRange;
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex | 
	                               vk::ShaderStageFlagBits::eFragment |
	                               vk::ShaderStageFlagBits::eCompute;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(RootConstants);
	
	// Pipeline layout with push constants only
	vk::PipelineLayoutCreateInfo layoutInfo;
	layoutInfo.setLayoutCount = 1;
	layoutInfo.pSetLayouts = &textureHeapLayout_;
	layoutInfo.pushConstantRangeCount = 1;
	layoutInfo.pPushConstantRanges = &pushConstantRange;
	
	layout_ = device_.createPipelineLayout(layoutInfo);
}

VulkanBindlessLayout::~VulkanBindlessLayout() {
	if (device_) {
		if (layout_) {
			device_.destroyPipelineLayout(layout_);
		}
		if (textureHeapLayout_) {
			device_.destroyDescriptorSetLayout(textureHeapLayout_);
		}
	}
}

VulkanBindlessLayout::VulkanBindlessLayout(VulkanBindlessLayout&& other) noexcept
	: device_(other.device_)
	, layout_(other.layout_)
	, textureHeapLayout_(other.textureHeapLayout_)
{
	other.device_ = nullptr;
	other.layout_ = nullptr;
	other.textureHeapLayout_ = nullptr;
}

VulkanBindlessLayout& VulkanBindlessLayout::operator=(VulkanBindlessLayout&& other) noexcept {
	if (this != &other) {
		if (device_) {
			if (layout_) device_.destroyPipelineLayout(layout_);
			if (textureHeapLayout_) device_.destroyDescriptorSetLayout(textureHeapLayout_);
		}
		
		device_ = other.device_;
		layout_ = other.layout_;
		textureHeapLayout_ = other.textureHeapLayout_;
		
		other.device_ = nullptr;
		other.layout_ = nullptr;
		other.textureHeapLayout_ = nullptr;
	}
	return *this;
}

