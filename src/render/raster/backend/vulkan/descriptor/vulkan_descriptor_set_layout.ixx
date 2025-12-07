export module synodic.soul.raster.backend.vulkan:descriptor_set_layout;

import std;
import vulkan_hpp;
import synodic.soul.transput;

// Binding description for layout creation
export struct DescriptorBinding {
	std::uint32_t binding = 0;
	vk::DescriptorType type = vk::DescriptorType::eUniformBuffer;
	std::uint32_t count = 1;
	vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAllGraphics;
	
	// Convenience constructors
	static DescriptorBinding UniformBuffer(std::uint32_t binding, 
		vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAllGraphics) 
	{
		return {binding, vk::DescriptorType::eUniformBuffer, 1, stages};
	}
	
	static DescriptorBinding StorageBuffer(std::uint32_t binding,
		vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAllGraphics)
	{
		return {binding, vk::DescriptorType::eStorageBuffer, 1, stages};
	}
	
	static DescriptorBinding CombinedImageSampler(std::uint32_t binding,
		vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eFragment)
	{
		return {binding, vk::DescriptorType::eCombinedImageSampler, 1, stages};
	}
	
	static DescriptorBinding SampledImage(std::uint32_t binding,
		vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eFragment)
	{
		return {binding, vk::DescriptorType::eSampledImage, 1, stages};
	}
	
	static DescriptorBinding Sampler(std::uint32_t binding,
		vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eFragment)
	{
		return {binding, vk::DescriptorType::eSampler, 1, stages};
	}
	
	static DescriptorBinding StorageImage(std::uint32_t binding,
		vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eCompute)
	{
		return {binding, vk::DescriptorType::eStorageImage, 1, stages};
	}
	
	// Create from reflection data
	static DescriptorBinding FromReflection(const synodic::soul::shader::ReflectedBinding& reflected)
	{
		DescriptorBinding binding;
		binding.binding = reflected.binding;
		binding.count = reflected.count;
		
		// Convert descriptor type
		using DT = synodic::soul::shader::DescriptorType;
		switch (reflected.type) {
			case DT::UNIFORM_BUFFER:
				binding.type = vk::DescriptorType::eUniformBuffer;
				break;
			case DT::STORAGE_BUFFER:
				binding.type = vk::DescriptorType::eStorageBuffer;
				break;
			case DT::COMBINED_IMAGE_SAMPLER:
				binding.type = vk::DescriptorType::eCombinedImageSampler;
				break;
			case DT::SAMPLED_IMAGE:
				binding.type = vk::DescriptorType::eSampledImage;
				break;
			case DT::STORAGE_IMAGE:
				binding.type = vk::DescriptorType::eStorageImage;
				break;
			case DT::SAMPLER:
				binding.type = vk::DescriptorType::eSampler;
				break;
			case DT::INPUT_ATTACHMENT:
				binding.type = vk::DescriptorType::eInputAttachment;
				break;
			default:
				binding.type = vk::DescriptorType::eUniformBuffer;
				break;
		}
		
		// Convert stage flags
		binding.stages = {};
		using SF = synodic::soul::shader::ShaderStageFlags;
		if (synodic::soul::shader::HasFlag(reflected.stages, SF::VERTEX))
			binding.stages |= vk::ShaderStageFlagBits::eVertex;
		if (synodic::soul::shader::HasFlag(reflected.stages, SF::FRAGMENT))
			binding.stages |= vk::ShaderStageFlagBits::eFragment;
		if (synodic::soul::shader::HasFlag(reflected.stages, SF::COMPUTE))
			binding.stages |= vk::ShaderStageFlagBits::eCompute;
		if (synodic::soul::shader::HasFlag(reflected.stages, SF::GEOMETRY))
			binding.stages |= vk::ShaderStageFlagBits::eGeometry;
		if (synodic::soul::shader::HasFlag(reflected.stages, SF::TESSELLATION_CONTROL))
			binding.stages |= vk::ShaderStageFlagBits::eTessellationControl;
		if (synodic::soul::shader::HasFlag(reflected.stages, SF::TESSELLATION_EVALUATION))
			binding.stages |= vk::ShaderStageFlagBits::eTessellationEvaluation;
		
		// Default to all graphics if no specific stages
		if (!binding.stages)
			binding.stages = vk::ShaderStageFlagBits::eAllGraphics;
		
		return binding;
	}
};

// Descriptor set layout - defines the structure of a descriptor set
export class VulkanDescriptorSetLayout {
public:
	VulkanDescriptorSetLayout() = default;
	
	VulkanDescriptorSetLayout(vk::Device device, std::span<const DescriptorBinding> bindings)
		: device_(device)
	{
		std::vector<vk::DescriptorSetLayoutBinding> vkBindings;
		vkBindings.reserve(bindings.size());
		
		for (const auto& binding : bindings) {
			vk::DescriptorSetLayoutBinding vkBinding;
			vkBinding.binding = binding.binding;
			vkBinding.descriptorType = binding.type;
			vkBinding.descriptorCount = binding.count;
			vkBinding.stageFlags = binding.stages;
			vkBinding.pImmutableSamplers = nullptr;
			vkBindings.push_back(vkBinding);
		}
		
		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.bindingCount = static_cast<std::uint32_t>(vkBindings.size());
		layoutInfo.pBindings = vkBindings.data();
		
		layout_ = device_.createDescriptorSetLayout(layoutInfo);
		bindings_.assign(bindings.begin(), bindings.end());
	}
	
	~VulkanDescriptorSetLayout() {
		if (layout_ && device_) {
			device_.destroyDescriptorSetLayout(layout_);
		}
	}
	
	VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
	VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;
	
	VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept
		: device_(other.device_)
		, layout_(other.layout_)
		, bindings_(std::move(other.bindings_))
	{
		other.layout_ = nullptr;
		other.device_ = nullptr;
	}
	
	VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& other) noexcept {
		if (this != &other) {
			if (layout_ && device_) {
				device_.destroyDescriptorSetLayout(layout_);
			}
			device_ = other.device_;
			layout_ = other.layout_;
			bindings_ = std::move(other.bindings_);
			other.layout_ = nullptr;
			other.device_ = nullptr;
		}
		return *this;
	}
	
	[[nodiscard]] vk::DescriptorSetLayout Handle() const { return layout_; }
	[[nodiscard]] const std::vector<DescriptorBinding>& Bindings() const { return bindings_; }

private:
	vk::Device device_;
	vk::DescriptorSetLayout layout_;
	std::vector<DescriptorBinding> bindings_;
};

// Pre-defined layouts for common use cases
export struct StandardDescriptorLayouts {
	// Material data (baseColor, metallic, roughness, ao, emissive)
	static std::vector<DescriptorBinding> Material() {
		return {
			DescriptorBinding::UniformBuffer(0, vk::ShaderStageFlagBits::eFragment)
		};
	}
	
	// Scene lighting (camera pos, ambient, lights array)
	static std::vector<DescriptorBinding> SceneLighting() {
		return {
			DescriptorBinding::UniformBuffer(0, vk::ShaderStageFlagBits::eFragment)
		};
	}
	
	// PBR material with textures
	static std::vector<DescriptorBinding> PBRMaterial() {
		return {
			DescriptorBinding::UniformBuffer(0, vk::ShaderStageFlagBits::eFragment),  // Material params
			DescriptorBinding::CombinedImageSampler(1),  // Albedo
			DescriptorBinding::CombinedImageSampler(2),  // Normal
			DescriptorBinding::CombinedImageSampler(3),  // Metallic-Roughness
			DescriptorBinding::CombinedImageSampler(4),  // AO
			DescriptorBinding::CombinedImageSampler(5),  // Emissive
		};
	}
	
	// Per-frame global data (view/projection, time, etc.)
	static std::vector<DescriptorBinding> PerFrame() {
		return {
			DescriptorBinding::UniformBuffer(0, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
		};
	}
	
	// Simple: just material + lighting UBOs (no textures)
	static std::vector<DescriptorBinding> SimplePBR() {
		return {
			DescriptorBinding::UniformBuffer(0, vk::ShaderStageFlagBits::eFragment),  // Material
			DescriptorBinding::UniformBuffer(1, vk::ShaderStageFlagBits::eFragment),  // Lighting
		};
	}
};
