
##############################################
#Acceleration

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			node.ixx
			lbvh_manager.ixx
    PRIVATE
			lbvh_manager.cpp
			node.cpp
)

add_subdirectory(cuda)
