
##############################################
#Window

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			monitor.ixx
			window.ixx
)

add_subdirectory(backend)
