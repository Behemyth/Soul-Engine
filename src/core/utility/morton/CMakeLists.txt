
##############################################
#Morton

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			morton.ixx
)

add_subdirectory(cuda)
