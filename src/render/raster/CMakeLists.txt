##############################################
#Rasterer

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES	
			types.ixx
			resource.ixx
			gpu_pointer.ixx
			frame_allocator.ixx
			barrier.ixx
			texture_heap.ixx
			depth_stencil_state.ixx
			blend_state.ixx
			commands.ixx
			device.ixx
			command_list.ixx
			module.ixx
	PRIVATE
		
)

add_subdirectory(backend)
