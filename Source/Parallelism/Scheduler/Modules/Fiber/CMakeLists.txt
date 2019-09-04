
##############################################
#Parallelism

option(BUILD_FIBER_SCHEDULER "Build the Fiber Scheduler backend" ON)


target_sources(${PROJECT_NAME}
    PRIVATE
        FiberProperties.cpp
        FiberProperties.h
        FiberSchedulerAlgorithm.cpp
        FiberSchedulerAlgorithm.h
        FiberSchedulerBackend.cpp
        FiberSchedulerBackend.h
)

target_compile_definitions(${PROJECT_NAME} 
    PRIVATE
        FIBER_SCHEDULER=BUILD_FIBER_SCHEDULER
)
