##############################################
# Mesh Resource Loading

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			mesh_error.ixx
			mesh_loader.ixx
)

if(BUILD_GLTF_LOADER)
	add_subdirectory(backend/gltf)
endif()

