export module synodic.soul.transput:shader_error;

import std;

export namespace synodic::soul::shader
{

	enum class ShaderErrorCode
	{
		SUCCESS = 0,
		COMPILATION_FAILED,
		INVALID_STAGE,
		INVALID_ENTRY_POINT,
		FILE_NOT_FOUND,
		INVALID_SOURCE,
		CACHE_ERROR,
		SLANG_INIT_FAILED,
		SLANG_COMPILE_FAILED,
		UNKNOWN_ERROR
	};

	class ShaderErrorCategory : public std::error_category
	{
	public:
		[[nodiscard]] const char* name() const noexcept override
		{
			return "soul_shader";
		}

		[[nodiscard]] std::string message(int condition) const override
		{
			switch (static_cast<ShaderErrorCode>(condition))
			{
				case ShaderErrorCode::SUCCESS :
					return "Success";
				case ShaderErrorCode::COMPILATION_FAILED :
					return "Shader compilation failed";
				case ShaderErrorCode::INVALID_STAGE :
					return "Invalid shader stage";
				case ShaderErrorCode::INVALID_ENTRY_POINT :
					return "Invalid entry point";
				case ShaderErrorCode::FILE_NOT_FOUND :
					return "Shader file not found";
				case ShaderErrorCode::INVALID_SOURCE :
					return "Invalid shader source";
				case ShaderErrorCode::CACHE_ERROR :
					return "Shader cache error";
				case ShaderErrorCode::SLANG_INIT_FAILED :
					return "Failed to initialize Slang compiler";
				case ShaderErrorCode::SLANG_COMPILE_FAILED :
					return "Slang compilation failed";
				case ShaderErrorCode::UNKNOWN_ERROR :
					return "Unknown shader error";
				default :
					return "Unrecognized error";
			}
		}
	};

	inline const ShaderErrorCategory& GetShaderErrorCategory() noexcept
	{
		static ShaderErrorCategory category;
		return category;
	}

	inline std::error_code MakeErrorCode(ShaderErrorCode code) noexcept
	{
		return {static_cast<int>(code), GetShaderErrorCategory()};
	}

	template<typename T>
	using ShaderResult = std::expected<T, std::error_code>;

	struct ShaderDiagnostic
	{
		enum class Severity
		{
			NOTE,
			WARNING,
			ERROR
		};

		Severity severity;
		std::string message;
		std::string filePath;
		int line {0};
		int column {0};
	};

	struct CompilationResult
	{
		std::vector<std::byte> spirvCode;
		std::vector<ShaderDiagnostic> diagnostics;
		bool hasErrors {false};
	};

}

template<>
struct std::is_error_code_enum<synodic::soul::shader::ShaderErrorCode> : std::true_type
{
};
