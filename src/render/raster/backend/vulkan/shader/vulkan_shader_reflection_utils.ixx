export module synodic.soul.raster.backend.vulkan:shader_reflection_utils;

import std;
import vulkan;
import synodic.soul.transput;

export namespace synodic::soul::shader
{

	// ============================================================================
	// Convert reflection types to Vulkan types
	// ============================================================================

	[[nodiscard]] constexpr vk::DescriptorType ToVulkanDescriptorType(DescriptorType type) noexcept
	{
		switch (type)
		{
			case DescriptorType::UNIFORM_BUFFER :
				return vk::DescriptorType::eUniformBuffer;
			case DescriptorType::STORAGE_BUFFER :
				return vk::DescriptorType::eStorageBuffer;
			case DescriptorType::COMBINED_IMAGE_SAMPLER :
				return vk::DescriptorType::eCombinedImageSampler;
			case DescriptorType::SAMPLED_IMAGE :
				return vk::DescriptorType::eSampledImage;
			case DescriptorType::STORAGE_IMAGE :
				return vk::DescriptorType::eStorageImage;
			case DescriptorType::SAMPLER :
				return vk::DescriptorType::eSampler;
			case DescriptorType::INPUT_ATTACHMENT :
				return vk::DescriptorType::eInputAttachment;
			case DescriptorType::ACCELERATION_STRUCTURE :
				return vk::DescriptorType::eAccelerationStructureKHR;
			default :
				return vk::DescriptorType::eUniformBuffer;
		}
	}

	[[nodiscard]] constexpr vk::ShaderStageFlags ToVulkanShaderStages(ShaderStageFlags flags) noexcept
	{
		vk::ShaderStageFlags result;

		if (HasFlag(flags, ShaderStageFlags::VERTEX))
		{
			result |= vk::ShaderStageFlagBits::eVertex;
		}
		if (HasFlag(flags, ShaderStageFlags::TESSELLATION_CONTROL))
		{
			result |= vk::ShaderStageFlagBits::eTessellationControl;
		}
		if (HasFlag(flags, ShaderStageFlags::TESSELLATION_EVALUATION))
		{
			result |= vk::ShaderStageFlagBits::eTessellationEvaluation;
		}
		if (HasFlag(flags, ShaderStageFlags::GEOMETRY))
		{
			result |= vk::ShaderStageFlagBits::eGeometry;
		}
		if (HasFlag(flags, ShaderStageFlags::FRAGMENT))
		{
			result |= vk::ShaderStageFlagBits::eFragment;
		}
		if (HasFlag(flags, ShaderStageFlags::COMPUTE))
		{
			result |= vk::ShaderStageFlagBits::eCompute;
		}
		if (HasFlag(flags, ShaderStageFlags::RAY_GEN))
		{
			result |= vk::ShaderStageFlagBits::eRaygenKHR;
		}
		if (HasFlag(flags, ShaderStageFlags::ANY_HIT))
		{
			result |= vk::ShaderStageFlagBits::eAnyHitKHR;
		}
		if (HasFlag(flags, ShaderStageFlags::CLOSEST_HIT))
		{
			result |= vk::ShaderStageFlagBits::eClosestHitKHR;
		}
		if (HasFlag(flags, ShaderStageFlags::MISS))
		{
			result |= vk::ShaderStageFlagBits::eMissKHR;
		}
		if (HasFlag(flags, ShaderStageFlags::INTERSECTION))
		{
			result |= vk::ShaderStageFlagBits::eIntersectionKHR;
		}
		if (HasFlag(flags, ShaderStageFlags::CALLABLE))
		{
			result |= vk::ShaderStageFlagBits::eCallableKHR;
		}
		if (HasFlag(flags, ShaderStageFlags::TASK))
		{
			result |= vk::ShaderStageFlagBits::eTaskEXT;
		}
		if (HasFlag(flags, ShaderStageFlags::MESH))
		{
			result |= vk::ShaderStageFlagBits::eMeshEXT;
		}

		return result;
	}

	[[nodiscard]] constexpr vk::Format ToVulkanVertexFormat(ScalarType scalar, std::uint32_t vectorSize) noexcept
	{
		switch (scalar)
		{
			case ScalarType::FLOAT32 :
				switch (vectorSize)
				{
					case 1 :
						return vk::Format::eR32Sfloat;
					case 2 :
						return vk::Format::eR32G32Sfloat;
					case 3 :
						return vk::Format::eR32G32B32Sfloat;
					case 4 :
						return vk::Format::eR32G32B32A32Sfloat;
				}
				break;

			case ScalarType::INT32 :
				switch (vectorSize)
				{
					case 1 :
						return vk::Format::eR32Sint;
					case 2 :
						return vk::Format::eR32G32Sint;
					case 3 :
						return vk::Format::eR32G32B32Sint;
					case 4 :
						return vk::Format::eR32G32B32A32Sint;
				}
				break;

			case ScalarType::UINT32 :
				switch (vectorSize)
				{
					case 1 :
						return vk::Format::eR32Uint;
					case 2 :
						return vk::Format::eR32G32Uint;
					case 3 :
						return vk::Format::eR32G32B32Uint;
					case 4 :
						return vk::Format::eR32G32B32A32Uint;
				}
				break;

			case ScalarType::FLOAT16 :
				switch (vectorSize)
				{
					case 1 :
						return vk::Format::eR16Sfloat;
					case 2 :
						return vk::Format::eR16G16Sfloat;
					case 3 :
						return vk::Format::eR16G16B16Sfloat;
					case 4 :
						return vk::Format::eR16G16B16A16Sfloat;
				}
				break;

			default :
				break;
		}

		return vk::Format::eR32G32B32A32Sfloat;
	}

	// ============================================================================
	// Build Vulkan structures from reflection data
	// ============================================================================

	/**
	 * @brief Build descriptor set layout bindings from reflection data
	 * @param reflection The shader reflection data
	 * @param setIndex The descriptor set index to extract bindings for
	 * @return Vector of Vulkan descriptor set layout bindings
	 */
	[[nodiscard]] inline std::vector<vk::DescriptorSetLayoutBinding>
		BuildDescriptorSetLayoutBindings(const ShaderReflectionData& reflection, std::uint32_t setIndex)
	{
		std::vector<vk::DescriptorSetLayoutBinding> bindings;

		for (const auto& binding: reflection.bindings)
		{
			if (binding.set != setIndex)
			{
				continue;
			}

			vk::DescriptorSetLayoutBinding vkBinding;
			vkBinding.binding			= binding.binding;
			vkBinding.descriptorType	= ToVulkanDescriptorType(binding.type);
			vkBinding.descriptorCount	= binding.count;
			vkBinding.stageFlags		= ToVulkanShaderStages(binding.stages);
			vkBinding.pImmutableSamplers = nullptr;

			bindings.push_back(vkBinding);
		}

		// Sort by binding number
		std::ranges::sort(bindings, {}, &vk::DescriptorSetLayoutBinding::binding);

		return bindings;
	}

	/**
	 * @brief Build push constant ranges from reflection data
	 * @param reflection The shader reflection data
	 * @return Vector of Vulkan push constant ranges
	 */
	[[nodiscard]] inline std::vector<vk::PushConstantRange>
		BuildPushConstantRanges(const ShaderReflectionData& reflection)
	{
		std::vector<vk::PushConstantRange> ranges;

		for (const auto& pc: reflection.pushConstants)
		{
			vk::PushConstantRange range;
			range.stageFlags = ToVulkanShaderStages(pc.stages);
			range.offset	 = pc.offset;
			range.size		 = pc.size;
			ranges.push_back(range);
		}

		return ranges;
	}

	/**
	 * @brief Build vertex input attribute descriptions from reflection data
	 * @param reflection The shader reflection data
	 * @param entryPointName The vertex shader entry point name
	 * @return Pair of (binding description, attribute descriptions)
	 */
	[[nodiscard]] inline std::pair<vk::VertexInputBindingDescription, std::vector<vk::VertexInputAttributeDescription>>
		BuildVertexInputDescriptions(const ShaderReflectionData& reflection, std::string_view entryPointName)
	{
		vk::VertexInputBindingDescription bindingDesc;
		std::vector<vk::VertexInputAttributeDescription> attributeDescs;

		// Find the vertex shader entry point
		const ReflectedEntryPoint* vertexEntry = nullptr;
		for (const auto& ep: reflection.entryPoints)
		{
			if (ep.name == entryPointName && HasFlag(ep.stage, ShaderStageFlags::VERTEX))
			{
				vertexEntry = &ep;
				break;
			}
		}

		if (!vertexEntry)
		{
			return {bindingDesc, attributeDescs};
		}

		// Calculate stride from inputs
		std::uint32_t stride = 0;
		for (const auto& input: vertexEntry->inputs)
		{
			stride = std::max(stride, input.offset + input.size);
		}

		bindingDesc.binding	  = 0;
		bindingDesc.stride	  = stride;
		bindingDesc.inputRate = vk::VertexInputRate::eVertex;

		// Build attribute descriptions
		for (const auto& input: vertexEntry->inputs)
		{
			vk::VertexInputAttributeDescription attrDesc;
			attrDesc.location = input.location;
			attrDesc.binding  = 0;
			attrDesc.format	  = ToVulkanVertexFormat(input.scalarType, input.vectorSize);
			attrDesc.offset	  = input.offset;
			attributeDescs.push_back(attrDesc);
		}

		// Sort by location
		std::ranges::sort(attributeDescs, {}, &vk::VertexInputAttributeDescription::location);

		return {bindingDesc, attributeDescs};
	}

	// ============================================================================
	// Reflection-aware pipeline layout builder
	// ============================================================================

	/**
	 * @brief Configuration for creating a pipeline layout from reflection
	 */
	struct ReflectedPipelineLayoutConfig
	{
		const ShaderReflectionData* reflection = nullptr;
		std::vector<vk::DescriptorSetLayout> additionalSetLayouts;
		bool includePushConstants = true;
	};

	/**
	 * @brief Create a pipeline layout from shader reflection
	 * @param device Vulkan device
	 * @param config Configuration with reflection data
	 * @return Pipeline layout handle
	 */
	[[nodiscard]] inline vk::PipelineLayout
		CreatePipelineLayoutFromReflection(vk::Device device, const ReflectedPipelineLayoutConfig& config)
	{
		std::vector<vk::PushConstantRange> pushConstantRanges;
		if (config.includePushConstants && config.reflection)
		{
			pushConstantRanges = BuildPushConstantRanges(*config.reflection);
		}

		vk::PipelineLayoutCreateInfo createInfo;
		createInfo.setLayoutCount		  = static_cast<std::uint32_t>(config.additionalSetLayouts.size());
		createInfo.pSetLayouts			  = config.additionalSetLayouts.data();
		createInfo.pushConstantRangeCount = static_cast<std::uint32_t>(pushConstantRanges.size());
		createInfo.pPushConstantRanges	  = pushConstantRanges.data();

		return device.createPipelineLayout(createInfo);
	}

	// ============================================================================
	// Validation helpers
	// ============================================================================

	/**
	 * @brief Validate that a descriptor set layout matches reflection data
	 * @param reflection The shader reflection data
	 * @param setIndex The descriptor set index
	 * @param layout The Vulkan descriptor set layout to validate
	 * @param bindings The bindings used to create the layout
	 * @return Validation result with any errors
	 */
	[[nodiscard]] inline LayoutValidationResult ValidateDescriptorSetLayout(
		const ShaderReflectionData& reflection,
		std::uint32_t setIndex,
		std::span<const vk::DescriptorSetLayoutBinding> bindings)
	{
		LayoutValidationResult result;

		auto expectedBindings = BuildDescriptorSetLayoutBindings(reflection, setIndex);

		// Check each expected binding exists in provided bindings
		for (const auto& expected: expectedBindings)
		{
			auto it = std::ranges::find_if(
				bindings,
				[&](const vk::DescriptorSetLayoutBinding& b)
				{
					return b.binding == expected.binding;
				});

			if (it == bindings.end())
			{
				result.valid = false;
				result.errors.push_back({LayoutValidationError::Type::BINDING_MISMATCH,
					std::format("Missing binding {} in set {}", expected.binding, setIndex),
					"",
					expected.binding,
					0});
			}
			else if (it->descriptorType != expected.descriptorType)
			{
				result.valid = false;
				result.errors.push_back({LayoutValidationError::Type::BINDING_MISMATCH,
					std::format(
						"Binding {} type mismatch: expected {}, got {}",
						expected.binding,
						static_cast<int>(expected.descriptorType),
						static_cast<int>(it->descriptorType)),
					"",
					static_cast<std::size_t>(expected.descriptorType),
					static_cast<std::size_t>(it->descriptorType)});
			}
		}

		return result;
	}

}
