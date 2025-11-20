
##############################################
#Acceleration

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			Node.ixx
			LBVHManager.ixx
    PRIVATE
			LBVHManager.cpp
			Node.cpp
)

add_subdirectory(cuda)
