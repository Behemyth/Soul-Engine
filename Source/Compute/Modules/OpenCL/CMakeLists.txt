##############################################
#Compute/OpenCL

option(BUILD_CPU_COMPUTE "Build the OpenCL Compute backend" ON)#TODO: Implement use of this option

find_package(OpenCL 1.2)

target_sources(${PROJECT_NAME}
    PRIVATE
        OpenCLBackend.cpp
        OpenCLBackend.h
        OpenCLBuffer.cpp
        OpenCLBuffer.h
        OpenCLDevice.cpp
        OpenCLDevice.h
)