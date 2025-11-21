export module synodic.soul.transput:shader_manager;

import std;
import :shader_error;
import :shader_stage;
import :slang_compiler;

export namespace synodic::soul::shader
{

	// TODO: Replace string type
	using ShaderID = std::string;

	struct CompiledShader
	{
		ShaderID id;
		std::vector<std::byte> spirvCode;
		ShaderCompileOptions options;
		std::filesystem::path sourcePath;
		std::chrono::system_clock::time_point compilationTime;
	};

	class ShaderManager
	{
	public:
		explicit ShaderManager(std::filesystem::path shaderDirectory = {}) :
			compiler_(SlangCompiler::Create()),
			shaderDirectory_(std::move(shaderDirectory))
		{
		}

		[[nodiscard]] ShaderResult<CompiledShader>
			CompileShader(const ShaderID& id, const std::filesystem::path& path, const ShaderCompileOptions& options)
		{
			if (!compiler_)
			{
				return std::unexpected(compiler_.error());
			}

			std::ifstream file(path, std::ios::in | std::ios::binary);
			if (!file)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::FILE_NOT_FOUND));
			}

			std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			return CompileShaderInternal(id, source, path, options);
		}

		[[nodiscard]] ShaderResult<CompiledShader>
			CompileShaderFromSource(const ShaderID& id, std::string_view source, const ShaderCompileOptions& options)
		{
			return CompileShaderInternal(id, source, {}, options);
		}

		[[nodiscard]] std::optional<std::reference_wrapper<const CompiledShader>> GetShader(const ShaderID& id) const
		{
			auto it = shaders_.find(id);
			if (it == shaders_.end())
			{
				return std::nullopt;
			}
			return std::cref(it->second);
		}

		[[nodiscard]] bool HasShader(const ShaderID& id) const noexcept
		{
			return shaders_.contains(id);
		}

		void RemoveShader(const ShaderID& id)
		{
			shaders_.erase(id);
		}

		void ClearShaders()
		{
			shaders_.clear();
		}

	private:
		[[nodiscard]] ShaderResult<CompiledShader> CompileShaderInternal(
			const ShaderID& id,
			std::string_view source,
			const std::filesystem::path& sourcePath,
			const ShaderCompileOptions& options)
		{
			if (!compiler_)
			{
				return std::unexpected(compiler_.error());
			}

			auto compileResult =
				compiler_->CompileFromSource(source, sourcePath.empty() ? id : sourcePath.string(), options);

			if (!compileResult)
			{
				return std::unexpected(compileResult.error());
			}

			auto& compilationResult = *compileResult;

			if (compilationResult.hasErrors)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::COMPILATION_FAILED));
			}

			CompiledShader shader {
				.id				 = id,
				.spirvCode		 = std::move(compilationResult.spirvCode),
				.options		 = options,
				.sourcePath		 = sourcePath,
				.compilationTime = std::chrono::system_clock::now()};

			shaders_[id] = shader;
			return shader;
		}

		ShaderResult<SlangCompiler> compiler_;
		std::filesystem::path shaderDirectory_;
		std::unordered_map<ShaderID, CompiledShader> shaders_;
	};

}
