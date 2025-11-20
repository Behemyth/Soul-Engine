target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			graph.ixx
			parameter.ixx
			builder.ixx
)

add_subdirectory(backend)
