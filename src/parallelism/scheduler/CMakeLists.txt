
##############################################
#Parallelism

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			parallelism.scheduler.ixx
			task_parameters.ixx
			scheduler_module.ixx
)

if(BUILD_FIBER_SCHEDULER)
	target_sources(soul-engine
		PUBLIC
			FILE_SET CXX_MODULES FILES
				modules/fiber/fiber.ixx
				modules/fiber/FiberProperties.ixx
				modules/fiber/FiberSchedulerAlgorithm.ixx
				modules/fiber/FiberSchedulerBackend.ixx
		PRIVATE
			modules/fiber/FiberProperties.cpp
			modules/fiber/FiberSchedulerAlgorithm.cpp
			modules/fiber/FiberSchedulerBackend.cpp
	)
endif()

target_compile_definitions(soul-engine
    PRIVATE
        FIBER_SCHEDULER=BUILD_FIBER_SCHEDULER
)
