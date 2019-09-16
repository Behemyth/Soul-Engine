##############################################
#CUDA Compute

include(CheckLanguage)

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

option(BUILD_CUDA_COMPUTE "Build the CUDA Compute backend" ${CUDA_FOUND})


##############################################
#Module

if(${BUILD_CUDA_COMPUTE})

	ModuleImplementation(
		TARGET CUDA_MODULE
	)

	set_target_properties(${CUDA_MODULE}
		PROPERTIES 
			CUDA_SEPARABLE_COMPILATION ON
	)

endif()