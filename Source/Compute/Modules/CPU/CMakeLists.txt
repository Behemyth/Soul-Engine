##############################################
#CPU Compute


##############################################
#Options

option(BUILD_CPU_COMPUTE "Build the CPU Compute backend" ON)


##############################################
#Module

if(${BUILD_CPU_COMPUTE})

    ModuleImplementation()

endif()