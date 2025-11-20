
##############################################
#Compute

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			compute.ixx
			compute_policy.ixx
			compute_backend.ixx
			compute_device.ixx
			compute_buffer.ixx
			compute_module.ixx
)

add_subdirectory(Modules)
