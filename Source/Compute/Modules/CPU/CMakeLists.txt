##############################################
#Compute/CPU

option(BUILD_CPU_COMPUTE "Build the OpenCL Compute backend" ON)#TODO: Implement use of this option

target_sources(${PROJECT_NAME}
    PRIVATE
        CPUBackend.cpp
        CPUBackend.h
        CPUBuffer.cpp
        CPUBuffer.h
        CPUDevice.cpp
        CPUDevice.h
)