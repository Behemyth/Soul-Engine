
##############################################
#Utility

target_sources(soul-engine
	PUBLIC
		FILE_SET CXX_MODULES FILES
			utility.ixx
)

add_subdirectory(morton)
add_subdirectory(property)
