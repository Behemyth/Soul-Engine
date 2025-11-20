
##############################################
#Input

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			display.input.ixx
			button.ixx
			input_set.ixx
			"input.ixx"
)

add_subdirectory(backend)
