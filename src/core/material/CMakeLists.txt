
##############################################
#Material

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			material.ixx
)

add_subdirectory(colour)
add_subdirectory(cuda)
add_subdirectory(texture)
