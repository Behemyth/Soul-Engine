export module synodic.soul.transput:shader_stage;

import std;

export namespace synodic::soul::shader
{

	enum class ShaderStage
	{
		VERTEX,
		TESSELLATION_CONTROL,
		TESSELLATION_EVALUATION,
		GEOMETRY,
		FRAGMENT,
		COMPUTE,
		RAY_GEN,
		ANY_HIT,
		CLOSEST_HIT,
		MISS,
		INTERSECTION,
		CALLABLE,
		TASK,
		MESH
	};

	[[nodiscard]] constexpr std::string_view ToString(ShaderStage stage) noexcept
	{
		switch (stage)
		{
			case ShaderStage::VERTEX:
				return "vertex";
			case ShaderStage::TESSELLATION_CONTROL:
				return "hull";
			case ShaderStage::TESSELLATION_EVALUATION:
				return "domain";
			case ShaderStage::GEOMETRY:
				return "geometry";
			case ShaderStage::FRAGMENT:
				return "fragment";
			case ShaderStage::COMPUTE:
				return "compute";
			case ShaderStage::RAY_GEN:
				return "raygeneration";
			case ShaderStage::ANY_HIT:
				return "anyhit";
			case ShaderStage::CLOSEST_HIT:
				return "closesthit";
			case ShaderStage::MISS:
				return "miss";
			case ShaderStage::INTERSECTION:
				return "intersection";
			case ShaderStage::CALLABLE:
				return "callable";
			case ShaderStage::TASK:
				return "task";
			case ShaderStage::MESH:
				return "mesh";
			default:
				return "unknown";
		}
	}

	// Shader source formats supported by backends
	enum class ShaderSourceFormat
	{
		GLSL,
		HLSL,
		SLANG,
		SPIRV  // Pre-compiled SPIR-V
	};

	// Compile options for shader compilation (backend-agnostic)
	struct ShaderCompileOptions
	{
		ShaderStage stage;
		ShaderSourceFormat sourceFormat{ShaderSourceFormat::SLANG};
		std::string entryPoint{"main"};
		std::vector<std::string> defines;
		std::vector<std::filesystem::path> includePaths;
		int optimizationLevel{2};
		bool generateDebugInfo{false};
	};

}
