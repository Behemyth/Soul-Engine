



##############################################
#Resource

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			resource.ixx
)

add_subdirectory(shader)
add_subdirectory(mesh)
add_subdirectory(font)



