
##############################################
#Parallelism

option(BUILD_DISPATCH_SCHEDULER "Build the Dispatch Scheduler backend" ON)


target_sources(${PROJECT_NAME}
    PRIVATE
        DispatchSchedulerBackend.cpp
        DispatchSchedulerBackend.h
)

target_compile_definitions(${PROJECT_NAME} 
    PRIVATE
        DISPATCH_SCHEDULER=BUILD_DISPATCH_SCHEDULER
)
