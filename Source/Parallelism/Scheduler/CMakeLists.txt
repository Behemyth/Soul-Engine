
##############################################
#Parallelism

target_sources(${PROJECT_NAME}
    PRIVATE
        Modules/Dispatch/DispatchSchedulerBackend.cpp
        Modules/Dispatch/DispatchSchedulerBackend.h
        Modules/Fiber/FiberProperties.cpp
        Modules/Fiber/FiberProperties.h
        Modules/Fiber/FiberSchedulerAlgorithm.cpp
        Modules/Fiber/FiberSchedulerAlgorithm.h
        Modules/Fiber/FiberSchedulerBackend.cpp
        Modules/Fiber/FiberSchedulerBackend.h
        SchedulerModule.cpp
        SchedulerModule.h
        TaskParameters.cpp
        TaskParameters.h
)

target_compile_definitions(${PROJECT_NAME} 
    PRIVATE
        FIBER_SCHEDULER=BUILD_FIBER_SCHEDULER
)
