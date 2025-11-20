
##############################################
# Transput

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			transput.ixx
)

add_subdirectory(resource)
