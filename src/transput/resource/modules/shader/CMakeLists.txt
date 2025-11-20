
##############################################
#Resource

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			transput.shader.ixx
			shader_resource.ixx
			shader_loader.ixx
			shader_asset.ixx
			modules/spirv/transput.spirv.ixx
			modules/spirv/spirv_resource.ixx
			modules/spirv/spirv_loader.ixx
			modules/spirv/spirv_asset.ixx
	PRIVATE
			ShaderAsset.cpp
			ShaderResource.cpp
			ShaderLoader.cpp
			modules/spirv/SPIRVAsset.cpp
			modules/spirv/SPIRVResource.cpp
			modules/spirv/SPIRVLoader.cpp
)
