
##############################################
#Fiber Scheduler

find_package(boost)

##############################################
#Options

option(BUILD_FIBER_SCHEDULER "Build the Fiber Scheduler backend" ${boost_FOUND})


##############################################
#Module

if(${BUILD_FIBER_SCHEDULER})

    ModuleImplementation(
        TARGET FIBER_MODULE
    )

    target_link_libraries(${FIBER_MODULE}
        PRIVATE
            boost::boost
    )

endif()
