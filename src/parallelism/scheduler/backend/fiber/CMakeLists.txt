target_sources(soul-engine
	PUBLIC
		FILE_SET CXX_MODULES FILES
			fiber.ixx
			properties.ixx
			algorithm.ixx
			scheduler.ixx
	PRIVATE
		properties.cpp
		algorithm.cpp
		scheduler.cpp
)

target_compile_definitions(soul-engine
    PRIVATE
        FIBER_SCHEDULER=BUILD_FIBER_SCHEDULER
)
