/**
 * @file shader_compiler.ixx
 * @brief Abstract interface for shader compilers
 * 
 * This provides a backend-agnostic interface for shader compilation.
 * Implementations include Slang, DXC, glslc, etc.
 */
export module synodic.soul.transput:shader_compiler;

import std;
import :shader_error;
import :shader_stage;
import :shader_types;

export namespace synodic::soul::shader
{

	/**
	 * @brief Abstract base class for shader compilers
	 * 
	 * Implementations must provide:
	 * - CompileFromFile: Compile shader from file path
	 * - CompileFromSource: Compile shader from source string
	 * - ReflectFromFile: Get reflection data from file
	 * - ReflectFromSource: Get reflection data from source
	 */
	class IShaderCompiler
	{
	public:
		virtual ~IShaderCompiler() = default;

		// Compilation
		[[nodiscard]] virtual ShaderResult<CompilationResult>
			CompileFromFile(const std::filesystem::path& path, const ShaderCompileOptions& options) = 0;

		[[nodiscard]] virtual ShaderResult<CompilationResult>
			CompileFromSource(std::string_view source, std::string_view sourceName, const ShaderCompileOptions& options) = 0;

		// Reflection
		[[nodiscard]] virtual ShaderResult<ShaderReflectionData>
			ReflectFromFile(const std::filesystem::path& path, std::span<const std::string> entryPoints = {}) = 0;

		[[nodiscard]] virtual ShaderResult<ShaderReflectionData>
			ReflectFromSource(std::string_view source, std::string_view sourceName, std::span<const std::string> entryPoints = {}) = 0;

		// Capabilities
		[[nodiscard]] virtual bool SupportsSourceFormat(ShaderSourceFormat format) const noexcept = 0;
		[[nodiscard]] virtual std::string_view BackendName() const noexcept = 0;

	protected:
		IShaderCompiler() = default;
		IShaderCompiler(const IShaderCompiler&) = default;
		IShaderCompiler(IShaderCompiler&&) = default;
		IShaderCompiler& operator=(const IShaderCompiler&) = default;
		IShaderCompiler& operator=(IShaderCompiler&&) = default;
	};

	/**
	 * @brief Type-erased shader compiler wrapper
	 * 
	 * Provides value semantics for IShaderCompiler implementations.
	 */
	class ShaderCompiler
	{
	public:
		ShaderCompiler() = default;

		explicit ShaderCompiler(std::unique_ptr<IShaderCompiler> impl)
			: impl_(std::move(impl))
		{
		}

		[[nodiscard]] bool IsValid() const noexcept { return impl_ != nullptr; }
		[[nodiscard]] explicit operator bool() const noexcept { return IsValid(); }

		[[nodiscard]] ShaderResult<CompilationResult>
			CompileFromFile(const std::filesystem::path& path, const ShaderCompileOptions& options)
		{
			if (!impl_)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::BACKEND_INIT_FAILED));
			}
			return impl_->CompileFromFile(path, options);
		}

		[[nodiscard]] ShaderResult<CompilationResult>
			CompileFromSource(std::string_view source, std::string_view sourceName, const ShaderCompileOptions& options)
		{
			if (!impl_)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::BACKEND_INIT_FAILED));
			}
			return impl_->CompileFromSource(source, sourceName, options);
		}

		[[nodiscard]] ShaderResult<ShaderReflectionData>
			ReflectFromFile(const std::filesystem::path& path, std::span<const std::string> entryPoints = {})
		{
			if (!impl_)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::BACKEND_INIT_FAILED));
			}
			return impl_->ReflectFromFile(path, entryPoints);
		}

		[[nodiscard]] ShaderResult<ShaderReflectionData>
			ReflectFromSource(std::string_view source, std::string_view sourceName, std::span<const std::string> entryPoints = {})
		{
			if (!impl_)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::BACKEND_INIT_FAILED));
			}
			return impl_->ReflectFromSource(source, sourceName, entryPoints);
		}

		[[nodiscard]] bool SupportsSourceFormat(ShaderSourceFormat format) const noexcept
		{
			return impl_ && impl_->SupportsSourceFormat(format);
		}

		[[nodiscard]] std::string_view BackendName() const noexcept
		{
			return impl_ ? impl_->BackendName() : "none";
		}

	private:
		std::unique_ptr<IShaderCompiler> impl_;
	};

}
