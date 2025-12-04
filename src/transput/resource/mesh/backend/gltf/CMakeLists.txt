##############################################
# GLTF Mesh Loader Backend

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			gltf_types.ixx
			gltf_loader.ixx
)

