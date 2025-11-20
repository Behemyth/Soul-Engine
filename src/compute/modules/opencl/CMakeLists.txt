##############################################
#Compute/OpenCL

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			compute.opencl.ixx
    PRIVATE
			OpenCLBackend.cpp
			OpenCLBuffer.cpp
			OpenCLDevice.cpp
)
