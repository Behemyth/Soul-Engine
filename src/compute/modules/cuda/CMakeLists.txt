##############################################
#Compute/CUDA

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			compute.cuda.ixx
    PRIVATE
			cuda_backend.cpp
			cuda_buffer.cpp
			cuda_device.cpp
)
