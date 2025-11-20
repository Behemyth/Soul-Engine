
##############################################
#GUI

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES	
			widget.ixx
			layout.ixx
			gui_module.ixx
)

add_subdirectory(backend)
