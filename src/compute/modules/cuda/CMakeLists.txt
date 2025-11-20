##############################################
#Compute/CUDA

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			compute.cuda.ixx
    PRIVATE
			CUDABackend.cpp
			CUDABuffer.cpp
			CUDADevice.cpp
)
