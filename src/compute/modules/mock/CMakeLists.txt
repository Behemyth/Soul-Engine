##############################################
#Compute/Mock

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			compute.mock.ixx
    PRIVATE
			MockBackend.cpp
			MockBuffer.cpp
			MockDevice.cpp
)
