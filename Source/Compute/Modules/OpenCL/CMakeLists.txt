
##############################################
#OpenCL Compute

find_package(OpenCL 1.2)

##############################################
#Options

option(BUILD_OPENCL_COMPUTE "Build the OpenCL Compute backend" ${OpenCL_FOUND})

##############################################
#Module

if(${BUILD_OPENCL_COMPUTE})

    ModuleImplementation(
        DEPENDENCIES
            OpenCL
    )

endif()