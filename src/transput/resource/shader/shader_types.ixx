module;
#include <cstdint>

export module synodic.soul.transput:shader_types;

import std;

export namespace synodic::soul::shader
{

	// ============================================================================
	// Descriptor/Resource Types (API-agnostic, maps to Vulkan/D3D12/Metal)
	// ============================================================================

	enum class DescriptorType : std::uint8_t
	{
		UNKNOWN = 0,
		UNIFORM_BUFFER,
		STORAGE_BUFFER,
		COMBINED_IMAGE_SAMPLER,
		SAMPLED_IMAGE,
		STORAGE_IMAGE,
		SAMPLER,
		INPUT_ATTACHMENT,
		ACCELERATION_STRUCTURE
	};

	[[nodiscard]] constexpr std::string_view ToString(DescriptorType type) noexcept
	{
		switch (type)
		{
			case DescriptorType::UNIFORM_BUFFER:
				return "UniformBuffer";
			case DescriptorType::STORAGE_BUFFER:
				return "StorageBuffer";
			case DescriptorType::COMBINED_IMAGE_SAMPLER:
				return "CombinedImageSampler";
			case DescriptorType::SAMPLED_IMAGE:
				return "SampledImage";
			case DescriptorType::STORAGE_IMAGE:
				return "StorageImage";
			case DescriptorType::SAMPLER:
				return "Sampler";
			case DescriptorType::INPUT_ATTACHMENT:
				return "InputAttachment";
			case DescriptorType::ACCELERATION_STRUCTURE:
				return "AccelerationStructure";
			default:
				return "Unknown";
		}
	}

	// ============================================================================
	// Shader Stage Flags (bitmask for multi-stage visibility)
	// ============================================================================

	enum class ShaderStageFlags : std::uint32_t
	{
		NONE                    = 0,
		VERTEX                  = 1 << 0,
		TESSELLATION_CONTROL    = 1 << 1,
		TESSELLATION_EVALUATION = 1 << 2,
		GEOMETRY                = 1 << 3,
		FRAGMENT                = 1 << 4,
		COMPUTE                 = 1 << 5,
		RAY_GEN                 = 1 << 6,
		ANY_HIT                 = 1 << 7,
		CLOSEST_HIT             = 1 << 8,
		MISS                    = 1 << 9,
		INTERSECTION            = 1 << 10,
		CALLABLE                = 1 << 11,
		TASK                    = 1 << 12,
		MESH                    = 1 << 13,
		ALL_GRAPHICS            = VERTEX | TESSELLATION_CONTROL | TESSELLATION_EVALUATION | GEOMETRY | FRAGMENT,
		ALL                     = 0xFFFFFFFF
	};

	[[nodiscard]] constexpr ShaderStageFlags operator|(ShaderStageFlags a, ShaderStageFlags b) noexcept
	{
		return static_cast<ShaderStageFlags>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	}

	[[nodiscard]] constexpr ShaderStageFlags operator&(ShaderStageFlags a, ShaderStageFlags b) noexcept
	{
		return static_cast<ShaderStageFlags>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
	}

	[[nodiscard]] constexpr bool HasFlag(ShaderStageFlags flags, ShaderStageFlags test) noexcept
	{
		return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(test)) != 0;
	}

	// ============================================================================
	// Scalar Types (for struct field reflection)
	// ============================================================================

	enum class ScalarType : std::uint8_t
	{
		UNKNOWN = 0,
		BOOL,
		INT8,
		INT16,
		INT32,
		INT64,
		UINT8,
		UINT16,
		UINT32,
		UINT64,
		FLOAT16,
		FLOAT32,
		FLOAT64
	};

	[[nodiscard]] constexpr std::size_t ScalarTypeSize(ScalarType type) noexcept
	{
		switch (type)
		{
			case ScalarType::BOOL:
			case ScalarType::INT8:
			case ScalarType::UINT8:
				return 1;
			case ScalarType::INT16:
			case ScalarType::UINT16:
			case ScalarType::FLOAT16:
				return 2;
			case ScalarType::INT32:
			case ScalarType::UINT32:
			case ScalarType::FLOAT32:
				return 4;
			case ScalarType::INT64:
			case ScalarType::UINT64:
			case ScalarType::FLOAT64:
				return 8;
			default:
				return 0;
		}
	}

	// ============================================================================
	// Reflected Binding Information
	// ============================================================================

	struct ReflectedBinding
	{
		std::string name;
		std::uint32_t set{0};
		std::uint32_t binding{0};
		std::uint32_t count{1};
		DescriptorType type{DescriptorType::UNKNOWN};
		ShaderStageFlags stages{ShaderStageFlags::NONE};
		std::size_t size{0};  // For buffers: size in bytes
	};

	// ============================================================================
	// Reflected Push Constant Range
	// ============================================================================

	struct ReflectedPushConstant
	{
		std::string name;
		std::uint32_t offset{0};
		std::uint32_t size{0};
		ShaderStageFlags stages{ShaderStageFlags::NONE};
	};

	// ============================================================================
	// Reflected Struct Field (for UBO/SSBO/Push Constant layouts)
	// ============================================================================

	struct ReflectedField
	{
		std::string name;
		std::string typeName;
		std::uint32_t offset{0};
		std::uint32_t size{0};
		std::uint32_t arrayCount{1};  // 1 for non-arrays, >1 for arrays, 0 for unbounded
		std::uint32_t matrixRows{0};
		std::uint32_t matrixColumns{0};
		ScalarType scalarType{ScalarType::UNKNOWN};
	};

	// ============================================================================
	// Reflected Struct Type
	// ============================================================================

	struct ReflectedStruct
	{
		std::string name;
		std::size_t size{0};
		std::size_t alignment{0};
		std::vector<ReflectedField> fields;
	};

	// ============================================================================
	// Reflected Vertex Input Attribute
	// ============================================================================

	struct ReflectedVertexAttribute
	{
		std::string name;
		std::string semantic;
		std::uint32_t location{0};
		std::uint32_t offset{0};
		std::uint32_t size{0};
		ScalarType scalarType{ScalarType::FLOAT32};
		std::uint32_t vectorSize{1};  // 1-4 components
	};

	// ============================================================================
	// Reflected Entry Point
	// ============================================================================

	struct ReflectedEntryPoint
	{
		std::string name;
		ShaderStageFlags stage{ShaderStageFlags::NONE};
		std::vector<ReflectedVertexAttribute> inputs;
		std::vector<ReflectedVertexAttribute> outputs;

		// Compute shader specific
		std::array<std::uint32_t, 3> workgroupSize{1, 1, 1};
	};

	// ============================================================================
	// Complete Shader Reflection Data (backend-agnostic)
	// ============================================================================

	struct ShaderReflectionData
	{
		std::string moduleName;
		std::vector<ReflectedEntryPoint> entryPoints;
		std::vector<ReflectedBinding> bindings;
		std::vector<ReflectedPushConstant> pushConstants;
		std::vector<ReflectedStruct> structs;

		// Convenience accessors
		[[nodiscard]] std::optional<std::reference_wrapper<const ReflectedBinding>>
			FindBinding(std::uint32_t set, std::uint32_t binding) const noexcept
		{
			for (const auto& b : bindings)
			{
				if (b.set == set && b.binding == binding)
				{
					return std::cref(b);
				}
			}
			return std::nullopt;
		}

		[[nodiscard]] std::optional<std::reference_wrapper<const ReflectedBinding>>
			FindBindingByName(std::string_view name) const noexcept
		{
			for (const auto& b : bindings)
			{
				if (b.name == name)
				{
					return std::cref(b);
				}
			}
			return std::nullopt;
		}

		[[nodiscard]] std::optional<std::reference_wrapper<const ReflectedStruct>>
			FindStruct(std::string_view name) const noexcept
		{
			for (const auto& s : structs)
			{
				if (s.name == name)
				{
					return std::cref(s);
				}
			}
			return std::nullopt;
		}

		[[nodiscard]] std::size_t TotalPushConstantSize() const noexcept
		{
			std::size_t total = 0;
			for (const auto& pc : pushConstants)
			{
				total = std::max(total, static_cast<std::size_t>(pc.offset + pc.size));
			}
			return total;
		}

		[[nodiscard]] std::uint32_t MaxDescriptorSet() const noexcept
		{
			std::uint32_t maxSet = 0;
			for (const auto& b : bindings)
			{
				maxSet = std::max(maxSet, b.set);
			}
			return maxSet;
		}
	};

	// ============================================================================
	// Layout Validation Result
	// ============================================================================

	struct LayoutValidationError
	{
		enum class Type
		{
			SIZE_MISMATCH,
			ALIGNMENT_MISMATCH,
			FIELD_OFFSET_MISMATCH,
			FIELD_MISSING,
			FIELD_TYPE_MISMATCH,
			BINDING_MISMATCH
		};

		Type type;
		std::string message;
		std::string fieldName;
		std::size_t expected{0};
		std::size_t actual{0};
	};

	struct LayoutValidationResult
	{
		bool valid{true};
		std::vector<LayoutValidationError> errors;

		[[nodiscard]] explicit operator bool() const noexcept
		{
			return valid;
		}
	};

}
