
##############################################
#Dispatch Scheduler


##############################################
#Options

option(BUILD_DISPATCH_SCHEDULER "Build the Dispatch Scheduler backend" ON)


##############################################
#Module

if(${BUILD_DISPATCH_SCHEDULER})

    ModuleImplementation()

endif()