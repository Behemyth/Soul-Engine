
##############################################
#Resource

target_sources(${PROJECT_NAME}
    PRIVATE
        Modules/SPIRV/SPIRVAsset.cpp
        Modules/SPIRV/SPIRVAsset.h
        Modules/SPIRV/SPIRVResource.cpp
        Modules/SPIRV/SPIRVResource.h
        Modules/SPIRV/SPIRVLoader.cpp
        Modules/SPIRV/SPIRVLoader.h
        ShaderAsset.cpp
        ShaderAsset.h
        ShaderResource.cpp
        ShaderResource.h
        ShaderLoader.h
        ShaderLoader.cpp
)
