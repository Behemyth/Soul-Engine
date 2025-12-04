##############################################
#Compute/OpenCL

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			compute.opencl.ixx
    PRIVATE
			opencl_backend.cpp
			opencl_buffer.cpp
			opencl_device.cpp
)
