##############################################
#Compute/Mock

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			compute.mock.ixx
    PRIVATE
			mock_backend.cpp
			mock_buffer.cpp
			mock_device.cpp
)
