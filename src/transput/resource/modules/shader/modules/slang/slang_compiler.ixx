module;
#include <slang-com-ptr.h>
#include <slang.h>

export module synodic.soul.transput:slang_compiler;

import std;
import :shader_error;
import :shader_stage;

export namespace synodic::soul::shader
{

	// TODO: Replace handle type
	struct SlangSessionOpaque;
	using SlangSessionHandle = SlangSessionOpaque*;

	// TODO: Replace handle type
	struct SlangRequestOpaque;
	using SlangRequestHandle = SlangRequestOpaque*;

	class SlangCompiler
	{
	public:
		[[nodiscard]] static ShaderResult<SlangCompiler> Create()
		{
			SlangCompiler compiler;

			SlangSession* session = spCreateSession(nullptr);
			if (!session)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::SLANG_INIT_FAILED));
			}

			compiler.session_ = reinterpret_cast<SlangSessionHandle>(session);
			return compiler;
		}

		~SlangCompiler()
		{
			if (session_)
			{
				auto* session = reinterpret_cast<SlangSession*>(session_);
				spDestroySession(session);
			}
		}

		SlangCompiler(const SlangCompiler&)			   = delete;
		SlangCompiler& operator=(const SlangCompiler&) = delete;

		SlangCompiler(SlangCompiler&& other) noexcept :
			session_(std::exchange(other.session_, nullptr))
		{
		}

		SlangCompiler& operator=(SlangCompiler&& other) noexcept
		{
			if (this != &other)
			{
				if (session_)
				{
					auto* session = reinterpret_cast<SlangSession*>(session_);
					spDestroySession(session);
				}
				session_ = std::exchange(other.session_, nullptr);
			}
			return *this;
		}

		[[nodiscard]] ShaderResult<CompilationResult>
			CompileFromFile(const std::filesystem::path& path, const ShaderCompileOptions& options)
		{
			std::ifstream file(path, std::ios::in | std::ios::binary);
			if (!file)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::FILE_NOT_FOUND));
			}

			std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			return CompileFromSource(source, path.string(), options);
		}

		[[nodiscard]] ShaderResult<CompilationResult>
			CompileFromSource(std::string_view source, std::string_view sourceName, const ShaderCompileOptions& options)
		{
			auto* session = reinterpret_cast<SlangSession*>(session_);

			SlangCompileRequest* request = spCreateCompileRequest(session);
			if (!request)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::SLANG_COMPILE_FAILED));
			}

			ConfigureRequest(request, options);

			const int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
			spAddTranslationUnitSourceString(request, translationUnitIndex, sourceName.data(), source.data());

			const int entryPointIndex = spAddEntryPoint(
				request,
				translationUnitIndex,
				options.entryPoint.c_str(),
				StageToSlangStage(options.stage));

			const SlangResult result = spCompile(request);

			CompilationResult compilationResult;
			ExtractDiagnostics(request, compilationResult);

			if (SLANG_FAILED(result))
			{
				spDestroyCompileRequest(request);
				return std::unexpected(MakeErrorCode(ShaderErrorCode::SLANG_COMPILE_FAILED));
			}

			size_t outSize		= 0;
			const void* outCode = spGetEntryPointCode(request, entryPointIndex, &outSize);

			if (!outCode || outSize == 0)
			{
				spDestroyCompileRequest(request);
				return std::unexpected(MakeErrorCode(ShaderErrorCode::SLANG_COMPILE_FAILED));
			}

			compilationResult.spirvCode.resize(outSize);
			std::memcpy(compilationResult.spirvCode.data(), outCode, outSize);

			spDestroyCompileRequest(request);
			return compilationResult;
		}

	private:
		SlangCompiler() = default;

		void ConfigureRequest(SlangCompileRequest* request, const ShaderCompileOptions& options)
		{
			const int targetIndex = spAddCodeGenTarget(request, SLANG_SPIRV);

			spSetTargetFlags(
				request,
				targetIndex,
				options.optimizationLevel > 0 ? SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY : 0);

			if (options.generateDebugInfo)
			{
				spSetDebugInfoLevel(request, SLANG_DEBUG_INFO_LEVEL_STANDARD);
			}

			for (const auto& includePath: options.includePaths)
			{
				spAddSearchPath(request, includePath.string().c_str());
			}

			for (const auto& define: options.defines)
			{
				const auto pos = define.find('=');
				if (pos != std::string::npos)
				{
					const auto name	 = define.substr(0, pos);
					const auto value = define.substr(pos + 1);
					spAddPreprocessorDefine(request, name.c_str(), value.c_str());
				}
				else
				{
					spAddPreprocessorDefine(request, define.c_str(), "1");
				}
			}
		}

		void ExtractDiagnostics(SlangCompileRequest* request, CompilationResult& result)
		{
			ISlangBlob* diagnosticsBlob = nullptr;
			if (SLANG_SUCCEEDED(spGetDiagnosticOutputBlob(request, &diagnosticsBlob)))
			{
				if (diagnosticsBlob)
				{
					const auto* diagnosticText = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
					if (diagnosticText && diagnosticText[0] != '\0')
					{
						ShaderDiagnostic diagnostic;
						diagnostic.message	= diagnosticText;
						diagnostic.severity = ShaderDiagnostic::Severity::NOTE;

						const std::string diagStr(diagnosticText);
						if (diagStr.find("error") != std::string::npos)
						{
							diagnostic.severity = ShaderDiagnostic::Severity::ERROR;
							result.hasErrors	= true;
						}
						else if (diagStr.find("warning") != std::string::npos)
						{
							diagnostic.severity = ShaderDiagnostic::Severity::WARNING;
						}

						result.diagnostics.push_back(std::move(diagnostic));
					}
					diagnosticsBlob->release();
				}
			}
		}

		[[nodiscard]] static SlangStage StageToSlangStage(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::VERTEX :
					return SLANG_STAGE_VERTEX;
				case ShaderStage::TESSELLATION_CONTROL :
					return SLANG_STAGE_HULL;
				case ShaderStage::TESSELLATION_EVALUATION :
					return SLANG_STAGE_DOMAIN;
				case ShaderStage::GEOMETRY :
					return SLANG_STAGE_GEOMETRY;
				case ShaderStage::FRAGMENT :
					return SLANG_STAGE_FRAGMENT;
				case ShaderStage::COMPUTE :
					return SLANG_STAGE_COMPUTE;
				case ShaderStage::RAY_GEN :
					return SLANG_STAGE_RAY_GENERATION;
				case ShaderStage::ANY_HIT :
					return SLANG_STAGE_ANY_HIT;
				case ShaderStage::CLOSEST_HIT :
					return SLANG_STAGE_CLOSEST_HIT;
				case ShaderStage::MISS :
					return SLANG_STAGE_MISS;
				case ShaderStage::INTERSECTION :
					return SLANG_STAGE_INTERSECTION;
				case ShaderStage::CALLABLE :
					return SLANG_STAGE_CALLABLE;
				case ShaderStage::TASK :
					return SLANG_STAGE_AMPLIFICATION;
				case ShaderStage::MESH :
					return SLANG_STAGE_MESH;
				default :
					return SLANG_STAGE_NONE;
			}
		}

		SlangSessionHandle session_ {nullptr};
	};

}
