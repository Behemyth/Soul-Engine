##############################################
# Main Module Interface

target_sources(soul-engine
	PUBLIC
		FILE_SET CXX_MODULES FILES
			core.ixx
			engine.ixx
			types.ixx
			parameters.ixx
			utility.ixx
			structures.ixx
)

##############################################
# Subdirectories

add_subdirectory(audio)
add_subdirectory(backend)
add_subdirectory(compute)
add_subdirectory(core)
add_subdirectory(display)
add_subdirectory(network)
add_subdirectory(parallelism)
add_subdirectory(physics)
add_subdirectory(render)
add_subdirectory(tracer)
add_subdirectory(transput)
