##############################################
# Mesh

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			vertex.ixx
			mesh.ixx
			primitives.ixx
			upload.ixx
			module.ixx
)

