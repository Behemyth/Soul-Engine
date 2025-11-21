
##############################################
# Render

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			render.ixx
)

add_subdirectory(raster)
add_subdirectory(graph)
