/**
 * @file slang_compiler.ixx
 * @brief Slang shader compiler backend implementation
 * 
 * Provides IShaderCompiler implementation using the Slang compiler.
 * Supports compilation to SPIR-V and reflection.
 */
module;
#include <slang-com-ptr.h>
#include <slang.h>

export module synodic.soul.transput:slang_compiler;

import std;
import :shader_error;
import :shader_stage;
import :shader_types;
import :shader_compiler;

export namespace synodic::soul::shader
{

	/**
	 * @brief Slang shader compiler implementation
	 */
	class SlangShaderCompiler final : public IShaderCompiler
	{
	public:
		[[nodiscard]] static ShaderResult<std::unique_ptr<SlangShaderCompiler>> Create()
		{
			SlangSession* session = spCreateSession(nullptr);
			if (!session)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::BACKEND_INIT_FAILED));
			}

			return std::unique_ptr<SlangShaderCompiler>(new SlangShaderCompiler(session));
		}

		~SlangShaderCompiler() override
		{
			if (session_)
			{
				spDestroySession(session_);
			}
		}

		SlangShaderCompiler(const SlangShaderCompiler&) = delete;
		SlangShaderCompiler& operator=(const SlangShaderCompiler&) = delete;

		SlangShaderCompiler(SlangShaderCompiler&& other) noexcept
			: session_(std::exchange(other.session_, nullptr))
		{
		}

		SlangShaderCompiler& operator=(SlangShaderCompiler&& other) noexcept
		{
			if (this != &other)
			{
				if (session_)
				{
					spDestroySession(session_);
				}
				session_ = std::exchange(other.session_, nullptr);
			}
			return *this;
		}

		// ========================================================================
		// IShaderCompiler interface
		// ========================================================================

		[[nodiscard]] ShaderResult<CompilationResult>
			CompileFromFile(const std::filesystem::path& path, const ShaderCompileOptions& options) override
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
			CompileFromSource(std::string_view source, std::string_view sourceName, const ShaderCompileOptions& options) override
		{
			SlangCompileRequest* request = spCreateCompileRequest(session_);
			if (!request)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::BACKEND_COMPILE_FAILED));
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
				return std::unexpected(MakeErrorCode(ShaderErrorCode::BACKEND_COMPILE_FAILED));
			}

			size_t outSize = 0;
			const void* outCode = spGetEntryPointCode(request, entryPointIndex, &outSize);

			if (!outCode || outSize == 0)
			{
				spDestroyCompileRequest(request);
				return std::unexpected(MakeErrorCode(ShaderErrorCode::BACKEND_COMPILE_FAILED));
			}

			compilationResult.spirvCode.resize(outSize);
			std::memcpy(compilationResult.spirvCode.data(), outCode, outSize);

			spDestroyCompileRequest(request);
			return compilationResult;
		}

		[[nodiscard]] ShaderResult<ShaderReflectionData>
			ReflectFromFile(const std::filesystem::path& path, std::span<const std::string> entryPoints) override
		{
			std::ifstream file(path, std::ios::in | std::ios::binary);
			if (!file)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::FILE_NOT_FOUND));
			}

			std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			return ReflectFromSource(source, path.string(), entryPoints);
		}

		[[nodiscard]] ShaderResult<ShaderReflectionData>
			ReflectFromSource(std::string_view source, std::string_view sourceName, std::span<const std::string> entryPoints) override
		{
			// Create new session for reflection (uses different API)
			slang::IGlobalSession* globalSession = nullptr;
			if (SLANG_FAILED(slang::createGlobalSession(&globalSession)))
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::BACKEND_INIT_FAILED));
			}

			Slang::ComPtr<slang::IGlobalSession> globalSessionGuard(globalSession);

			// Create session with SPIRV target
			slang::SessionDesc sessionDesc = {};
			slang::TargetDesc targetDesc = {};
			targetDesc.format = SLANG_SPIRV;
			targetDesc.profile = globalSession->findProfile("spirv_1_5");
			sessionDesc.targets = &targetDesc;
			sessionDesc.targetCount = 1;

			slang::ISession* session = nullptr;
			if (SLANG_FAILED(globalSession->createSession(sessionDesc, &session)))
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::BACKEND_INIT_FAILED));
			}

			Slang::ComPtr<slang::ISession> sessionGuard(session);

			// Load module from source
			slang::IBlob* diagnosticsBlob = nullptr;
			slang::IModule* module = session->loadModuleFromSourceString(
				sourceName.data(),
				sourceName.data(),
				source.data(),
				&diagnosticsBlob);

			if (!module)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::REFLECTION_FAILED));
			}

			Slang::ComPtr<slang::IModule> moduleGuard(module);

			// Build component type with entry points
			std::vector<slang::IComponentType*> components;
			components.push_back(module);

			// Find and add entry points
			for (const auto& entryPointName : entryPoints)
			{
				slang::IEntryPoint* entryPoint = nullptr;
				if (SLANG_SUCCEEDED(module->findEntryPointByName(entryPointName.c_str(), &entryPoint)))
				{
					components.push_back(entryPoint);
				}
			}

			// If no entry points specified, find all
			if (entryPoints.empty())
			{
				const int entryPointCount = module->getDefinedEntryPointCount();
				for (int i = 0; i < entryPointCount; ++i)
				{
					slang::IEntryPoint* entryPoint = nullptr;
					if (SLANG_SUCCEEDED(module->getDefinedEntryPoint(i, &entryPoint)))
					{
						components.push_back(entryPoint);
					}
				}
			}

			// Create composite component type
			slang::IComponentType* composedProgram = nullptr;
			if (SLANG_FAILED(session->createCompositeComponentType(
				components.data(),
				components.size(),
				&composedProgram,
				&diagnosticsBlob)))
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::REFLECTION_FAILED));
			}

			Slang::ComPtr<slang::IComponentType> programGuard(composedProgram);

			// Link the program
			slang::IComponentType* linkedProgram = nullptr;
			if (SLANG_FAILED(composedProgram->link(&linkedProgram, &diagnosticsBlob)))
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::REFLECTION_FAILED));
			}

			Slang::ComPtr<slang::IComponentType> linkedGuard(linkedProgram);

			// Get program layout
			slang::ProgramLayout* programLayout = linkedProgram->getLayout(0);
			if (!programLayout)
			{
				return std::unexpected(MakeErrorCode(ShaderErrorCode::REFLECTION_FAILED));
			}

			return ExtractReflectionData(programLayout, sourceName);
		}

		[[nodiscard]] bool SupportsSourceFormat(ShaderSourceFormat format) const noexcept override
		{
			return format == ShaderSourceFormat::SLANG ||
			       format == ShaderSourceFormat::HLSL;
		}

		[[nodiscard]] std::string_view BackendName() const noexcept override
		{
			return "Slang";
		}

	private:
		explicit SlangShaderCompiler(SlangSession* session)
			: session_(session)
		{
		}

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

			for (const auto& includePath : options.includePaths)
			{
				spAddSearchPath(request, includePath.string().c_str());
			}

			for (const auto& define : options.defines)
			{
				const auto pos = define.find('=');
				if (pos != std::string::npos)
				{
					const auto name = define.substr(0, pos);
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
						diagnostic.message = diagnosticText;
						diagnostic.severity = ShaderDiagnostic::Severity::NOTE;

						const std::string diagStr(diagnosticText);
						if (diagStr.find("error") != std::string::npos)
						{
							diagnostic.severity = ShaderDiagnostic::Severity::ERROR;
							result.hasErrors = true;
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
				case ShaderStage::VERTEX:
					return SLANG_STAGE_VERTEX;
				case ShaderStage::TESSELLATION_CONTROL:
					return SLANG_STAGE_HULL;
				case ShaderStage::TESSELLATION_EVALUATION:
					return SLANG_STAGE_DOMAIN;
				case ShaderStage::GEOMETRY:
					return SLANG_STAGE_GEOMETRY;
				case ShaderStage::FRAGMENT:
					return SLANG_STAGE_FRAGMENT;
				case ShaderStage::COMPUTE:
					return SLANG_STAGE_COMPUTE;
				case ShaderStage::RAY_GEN:
					return SLANG_STAGE_RAY_GENERATION;
				case ShaderStage::ANY_HIT:
					return SLANG_STAGE_ANY_HIT;
				case ShaderStage::CLOSEST_HIT:
					return SLANG_STAGE_CLOSEST_HIT;
				case ShaderStage::MISS:
					return SLANG_STAGE_MISS;
				case ShaderStage::INTERSECTION:
					return SLANG_STAGE_INTERSECTION;
				case ShaderStage::CALLABLE:
					return SLANG_STAGE_CALLABLE;
				case ShaderStage::TASK:
					return SLANG_STAGE_AMPLIFICATION;
				case ShaderStage::MESH:
					return SLANG_STAGE_MESH;
				default:
					return SLANG_STAGE_NONE;
			}
		}

		// ========================================================================
		// Reflection helpers
		// ========================================================================

		[[nodiscard]] static ShaderReflectionData
			ExtractReflectionData(slang::ProgramLayout* programLayout, std::string_view moduleName)
		{
			ShaderReflectionData data;
			data.moduleName = moduleName;

			ExtractGlobalParameters(programLayout, data);

			const int entryPointCount = static_cast<int>(programLayout->getEntryPointCount());
			for (int i = 0; i < entryPointCount; ++i)
			{
				auto* entryPointLayout = programLayout->getEntryPointByIndex(i);
				if (entryPointLayout)
				{
					data.entryPoints.push_back(ExtractEntryPoint(entryPointLayout));
				}
			}

			return data;
		}

		static void ExtractGlobalParameters(slang::ProgramLayout* programLayout, ShaderReflectionData& data)
		{
			auto* globalScope = programLayout->getGlobalParamsVarLayout();
			if (!globalScope) return;

			auto* typeLayout = globalScope->getTypeLayout();
			if (!typeLayout) return;

			switch (typeLayout->getKind())
			{
				case slang::TypeReflection::Kind::Struct:
					ExtractStructBindings(typeLayout, data, ShaderStageFlags::ALL_GRAPHICS);
					break;

				case slang::TypeReflection::Kind::ConstantBuffer:
				case slang::TypeReflection::Kind::ParameterBlock:
				{
					auto* elementVarLayout = typeLayout->getElementVarLayout();
					if (elementVarLayout)
					{
						auto* elementTypeLayout = elementVarLayout->getTypeLayout();
						if (elementTypeLayout && elementTypeLayout->getKind() == slang::TypeReflection::Kind::Struct)
						{
							ExtractStructBindings(elementTypeLayout, data, ShaderStageFlags::ALL_GRAPHICS);
						}
					}
				}
				break;

				default:
					break;
			}
		}

		static void ExtractStructBindings(
			slang::TypeLayoutReflection* typeLayout,
			ShaderReflectionData& data,
			ShaderStageFlags stages)
		{
			const int fieldCount = static_cast<int>(typeLayout->getFieldCount());
			for (int i = 0; i < fieldCount; ++i)
			{
				auto* fieldLayout = typeLayout->getFieldByIndex(i);
				if (!fieldLayout) continue;

				auto* fieldTypeLayout = fieldLayout->getTypeLayout();
				if (!fieldTypeLayout) continue;

				auto kind = fieldTypeLayout->getKind();

				if (kind == slang::TypeReflection::Kind::Resource ||
					kind == slang::TypeReflection::Kind::SamplerState ||
					kind == slang::TypeReflection::Kind::ConstantBuffer ||
					kind == slang::TypeReflection::Kind::ParameterBlock ||
					kind == slang::TypeReflection::Kind::TextureBuffer ||
					kind == slang::TypeReflection::Kind::ShaderStorageBuffer)
				{
					ReflectedBinding binding;
					binding.name = fieldLayout->getName() ? fieldLayout->getName() : "";
					binding.stages = stages;
					binding.binding = static_cast<std::uint32_t>(
						fieldLayout->getOffset(slang::ParameterCategory::DescriptorTableSlot));
					binding.set = static_cast<std::uint32_t>(
						fieldLayout->getBindingSpace(slang::ParameterCategory::DescriptorTableSlot));
					binding.type = DetermineDescriptorType(fieldTypeLayout);

					if (kind == slang::TypeReflection::Kind::ConstantBuffer ||
						kind == slang::TypeReflection::Kind::ShaderStorageBuffer)
					{
						auto* elementTypeLayout = fieldTypeLayout->getElementTypeLayout();
						if (elementTypeLayout)
						{
							binding.size = elementTypeLayout->getSize();
							if (elementTypeLayout->getKind() == slang::TypeReflection::Kind::Struct)
							{
								auto structInfo = ExtractStructType(elementTypeLayout);
								structInfo.name = binding.name + "_Type";
								data.structs.push_back(std::move(structInfo));
							}
						}
					}

					data.bindings.push_back(std::move(binding));
				}
				else if (kind == slang::TypeReflection::Kind::Struct)
				{
					ExtractStructBindings(fieldTypeLayout, data, stages);
				}
			}
		}

		[[nodiscard]] static ReflectedStruct ExtractStructType(slang::TypeLayoutReflection* typeLayout)
		{
			ReflectedStruct result;
			result.name = typeLayout->getName() ? typeLayout->getName() : "UnnamedStruct";
			result.size = typeLayout->getSize();
			result.alignment = typeLayout->getAlignment();

			const int fieldCount = static_cast<int>(typeLayout->getFieldCount());
			for (int i = 0; i < fieldCount; ++i)
			{
				auto* fieldLayout = typeLayout->getFieldByIndex(i);
				if (!fieldLayout) continue;

				ReflectedField field;
				field.name = fieldLayout->getName() ? fieldLayout->getName() : "";
				field.offset = static_cast<std::uint32_t>(fieldLayout->getOffset());

				auto* fieldTypeLayout = fieldLayout->getTypeLayout();
				if (fieldTypeLayout)
				{
					field.size = static_cast<std::uint32_t>(fieldTypeLayout->getSize());
					field.typeName = fieldTypeLayout->getName() ? fieldTypeLayout->getName() : "";

					if (fieldTypeLayout->getKind() == slang::TypeReflection::Kind::Array)
					{
						field.arrayCount = static_cast<std::uint32_t>(fieldTypeLayout->getElementCount());
						auto* elementType = fieldTypeLayout->getElementTypeLayout();
						if (elementType)
						{
							field.typeName = elementType->getName() ? elementType->getName() : "";
						}
					}

					if (fieldTypeLayout->getKind() == slang::TypeReflection::Kind::Matrix)
					{
						field.matrixRows = static_cast<std::uint32_t>(fieldTypeLayout->getRowCount());
						field.matrixColumns = static_cast<std::uint32_t>(fieldTypeLayout->getColumnCount());
					}

					if (fieldTypeLayout->getKind() == slang::TypeReflection::Kind::Scalar ||
						fieldTypeLayout->getKind() == slang::TypeReflection::Kind::Vector)
					{
						field.scalarType = ConvertScalarType(fieldTypeLayout->getScalarType());
					}
				}

				result.fields.push_back(std::move(field));
			}

			return result;
		}

		[[nodiscard]] static ReflectedEntryPoint ExtractEntryPoint(slang::EntryPointReflection* entryPointLayout)
		{
			ReflectedEntryPoint result;
			result.name = entryPointLayout->getName() ? entryPointLayout->getName() : "";
			result.stage = ConvertStage(entryPointLayout->getStage());

			if (entryPointLayout->getStage() == SLANG_STAGE_COMPUTE)
			{
				SlangUInt sizes[3];
				entryPointLayout->getComputeThreadGroupSize(3, sizes);
				result.workgroupSize = {
					static_cast<std::uint32_t>(sizes[0]),
					static_cast<std::uint32_t>(sizes[1]),
					static_cast<std::uint32_t>(sizes[2])
				};
			}

			auto* varLayout = entryPointLayout->getVarLayout();
			if (varLayout)
			{
				auto* typeLayout = varLayout->getTypeLayout();
				if (typeLayout)
				{
					ExtractVaryingParameters(typeLayout, result);
				}
			}

			return result;
		}

		static void ExtractVaryingParameters(slang::TypeLayoutReflection* typeLayout, ReflectedEntryPoint& entryPoint)
		{
			if (typeLayout->getKind() == slang::TypeReflection::Kind::Struct)
			{
				const int fieldCount = static_cast<int>(typeLayout->getFieldCount());
				for (int i = 0; i < fieldCount; ++i)
				{
					auto* fieldLayout = typeLayout->getFieldByIndex(i);
					if (!fieldLayout) continue;

					const int categoryCount = static_cast<int>(fieldLayout->getCategoryCount());
					for (int c = 0; c < categoryCount; ++c)
					{
						auto category = fieldLayout->getCategoryByIndex(c);
						if (category == slang::ParameterCategory::VaryingInput)
						{
							entryPoint.inputs.push_back(ExtractVertexAttribute(fieldLayout, true));
						}
						else if (category == slang::ParameterCategory::VaryingOutput)
						{
							entryPoint.outputs.push_back(ExtractVertexAttribute(fieldLayout, false));
						}
					}
				}
			}
		}

		[[nodiscard]] static ReflectedVertexAttribute
			ExtractVertexAttribute(slang::VariableLayoutReflection* varLayout, bool isInput)
		{
			ReflectedVertexAttribute attr;
			attr.name = varLayout->getName() ? varLayout->getName() : "";
			attr.semantic = varLayout->getSemanticName() ? varLayout->getSemanticName() : "";
			attr.location = static_cast<std::uint32_t>(
				varLayout->getOffset(isInput ? slang::ParameterCategory::VaryingInput
				                             : slang::ParameterCategory::VaryingOutput));

			auto* typeLayout = varLayout->getTypeLayout();
			if (typeLayout)
			{
				attr.size = static_cast<std::uint32_t>(typeLayout->getSize());
				attr.scalarType = ConvertScalarType(typeLayout->getScalarType());

				if (typeLayout->getKind() == slang::TypeReflection::Kind::Vector)
				{
					attr.vectorSize = static_cast<std::uint32_t>(typeLayout->getElementCount());
				}
				else
				{
					attr.vectorSize = 1;
				}
			}

			return attr;
		}

		[[nodiscard]] static DescriptorType DetermineDescriptorType(slang::TypeLayoutReflection* typeLayout)
		{
			auto kind = typeLayout->getKind();

			switch (kind)
			{
				case slang::TypeReflection::Kind::ConstantBuffer:
					return DescriptorType::UNIFORM_BUFFER;

				case slang::TypeReflection::Kind::ShaderStorageBuffer:
					return DescriptorType::STORAGE_BUFFER;

				case slang::TypeReflection::Kind::SamplerState:
					return DescriptorType::SAMPLER;

				case slang::TypeReflection::Kind::Resource:
				{
					auto shape = typeLayout->getResourceShape();
					auto access = typeLayout->getResourceAccess();

					if (shape & SLANG_TEXTURE_1D || shape & SLANG_TEXTURE_2D ||
						shape & SLANG_TEXTURE_3D || shape & SLANG_TEXTURE_CUBE)
					{
						if (access == SLANG_RESOURCE_ACCESS_READ_WRITE ||
							access == SLANG_RESOURCE_ACCESS_WRITE)
						{
							return DescriptorType::STORAGE_IMAGE;
						}
						return DescriptorType::COMBINED_IMAGE_SAMPLER;
					}

					if (shape & SLANG_STRUCTURED_BUFFER)
					{
						if (access == SLANG_RESOURCE_ACCESS_READ_WRITE ||
							access == SLANG_RESOURCE_ACCESS_WRITE)
						{
							return DescriptorType::STORAGE_BUFFER;
						}
						return DescriptorType::UNIFORM_BUFFER;
					}
				}
				break;

				default:
					break;
			}

			return DescriptorType::UNKNOWN;
		}

		[[nodiscard]] static ScalarType ConvertScalarType(slang::TypeReflection::ScalarType slangType)
		{
			switch (slangType)
			{
				case slang::TypeReflection::ScalarType::Bool: return ScalarType::BOOL;
				case slang::TypeReflection::ScalarType::Int8: return ScalarType::INT8;
				case slang::TypeReflection::ScalarType::Int16: return ScalarType::INT16;
				case slang::TypeReflection::ScalarType::Int32: return ScalarType::INT32;
				case slang::TypeReflection::ScalarType::Int64: return ScalarType::INT64;
				case slang::TypeReflection::ScalarType::UInt8: return ScalarType::UINT8;
				case slang::TypeReflection::ScalarType::UInt16: return ScalarType::UINT16;
				case slang::TypeReflection::ScalarType::UInt32: return ScalarType::UINT32;
				case slang::TypeReflection::ScalarType::UInt64: return ScalarType::UINT64;
				case slang::TypeReflection::ScalarType::Float16: return ScalarType::FLOAT16;
				case slang::TypeReflection::ScalarType::Float32: return ScalarType::FLOAT32;
				case slang::TypeReflection::ScalarType::Float64: return ScalarType::FLOAT64;
				default: return ScalarType::UNKNOWN;
			}
		}

		[[nodiscard]] static ShaderStageFlags ConvertStage(SlangStage stage)
		{
			switch (stage)
			{
				case SLANG_STAGE_VERTEX: return ShaderStageFlags::VERTEX;
				case SLANG_STAGE_HULL: return ShaderStageFlags::TESSELLATION_CONTROL;
				case SLANG_STAGE_DOMAIN: return ShaderStageFlags::TESSELLATION_EVALUATION;
				case SLANG_STAGE_GEOMETRY: return ShaderStageFlags::GEOMETRY;
				case SLANG_STAGE_FRAGMENT: return ShaderStageFlags::FRAGMENT;
				case SLANG_STAGE_COMPUTE: return ShaderStageFlags::COMPUTE;
				case SLANG_STAGE_RAY_GENERATION: return ShaderStageFlags::RAY_GEN;
				case SLANG_STAGE_ANY_HIT: return ShaderStageFlags::ANY_HIT;
				case SLANG_STAGE_CLOSEST_HIT: return ShaderStageFlags::CLOSEST_HIT;
				case SLANG_STAGE_MISS: return ShaderStageFlags::MISS;
				case SLANG_STAGE_INTERSECTION: return ShaderStageFlags::INTERSECTION;
				case SLANG_STAGE_CALLABLE: return ShaderStageFlags::CALLABLE;
				case SLANG_STAGE_AMPLIFICATION: return ShaderStageFlags::TASK;
				case SLANG_STAGE_MESH: return ShaderStageFlags::MESH;
				default: return ShaderStageFlags::NONE;
			}
		}

		SlangSession* session_{nullptr};
	};

	/**
	 * @brief Create a Slang shader compiler
	 * @return ShaderCompiler wrapping a SlangShaderCompiler implementation
	 */
	[[nodiscard]] inline ShaderResult<ShaderCompiler> CreateSlangCompiler()
	{
		auto result = SlangShaderCompiler::Create();
		if (!result)
		{
			return std::unexpected(result.error());
		}
		return ShaderCompiler(std::move(*result));
	}

}
