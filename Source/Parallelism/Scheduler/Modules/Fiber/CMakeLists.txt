
##############################################
#Fiber Scheduler


##############################################
#Options

option(BUILD_FIBER_SCHEDULER "Build the Fiber Scheduler backend" ON)


##############################################
#Module

if(${BUILD_FIBER_SCHEDULER})

    ModuleImplementation()
    
endif()
