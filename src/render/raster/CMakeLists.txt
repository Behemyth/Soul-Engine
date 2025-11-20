##############################################
#Rasterer

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES	
			types.ixx
			resource.ixx
			commands.ixx
			device.ixx
			command_list.ixx
			module.ixx
	PRIVATE
		
)

add_subdirectory(backend)
