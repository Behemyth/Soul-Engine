cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

##############################################
#Compute Platforms

if(BUILD_CPU_COMPUTE)

    add_subdirectory("CPU")

endif()

# Mock backend for testing
add_subdirectory("Mock")

if(BUILD_CUDA_COMPUTE)

    add_subdirectory("CUDA")

endif()


if(BUILD_OPENCL_COMPUTE)

    add_subdirectory("OpenCL")

endif()

target_compile_definitions(soul-engine
    PRIVATE
        CUDA_COMPUTE=BUILD_CUDA_COMPUTE
        OPENCL_COMPUTE=BUILD_OPENCL_COMPUTE
)
