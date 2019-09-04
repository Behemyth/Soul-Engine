##############################################
#CUDA Compute

check_language(CUDA)

#`CMAKE_CUDA_COMPILER` will point to nvcc or be `NOTFOUND`
if(CMAKE_CUDA_COMPILER)

	enable_language(CUDA)
    set(CUDA_FOUND True)

else()

    set(CUDA_FOUND False)

endif()

##############################################
#Options

option(BUILD_CUDA_COMPUTE "Build the CUDA Compute backend" ${CUDA_FOUND})#TODO: Implement use of this option


target_sources(${PROJECT_NAME}
    PRIVATE
        CUDABackend.cpp
        CUDABackend.h
        CUDABuffer.cpp
        CUDABuffer.h
        CUDADevice.cpp
        CUDADevice.h
)

set_target_properties(${PROJECT_NAME}
	PROPERTIES 
		CUDA_SEPARABLE_COMPILATION ON
)