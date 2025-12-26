target_sources(soul-engine
	PUBLIC
		FILE_SET CXX_MODULES FILES
			vulkan.ixx
			vulkan_error.ixx
			vulkan_allocator.ixx
			vulkan_instance.ixx
			vulkan_raster_backend.ixx
			vulkan_fence.ixx
			vulkan_semaphore.ixx
			vulkan_framebuffer.ixx
			vulkan_render_pass.ixx
			vulkan_shader.ixx
			vulkan_sub_pass.ixx
			# Bindless resources (No Graphics API pattern)
			vulkan_texture_heap.ixx
			vulkan_sampler_heap.ixx
			vulkan_gpu_allocator.ixx
			vulkan_bindless_layout.ixx
			# Device
			device/vulkan_physical_device.ixx
			device/vulkan_queue.ixx
			device/vulkan_device.ixx
			# Commands
			command/vulkan_command_pool.ixx
			command/vulkan_command_buffer.ixx
			# Buffers
			buffer/vulkan_buffer.ixx
			# Pipelines
			pipeline/vulkan_pipeline.ixx
			pipeline/vulkan_pipeline_layout.ixx
			pipeline/vulkan_pipeline_cache.ixx
			# Surface
			surface/vulkan_surface.ixx
			surface/vulkan_swap_chain.ixx
			surface/vulkan_frame.ixx
			# Shader
			shader/vulkan_shader_reflection_utils.ixx
	PRIVATE
		vulkan_allocator.cpp
		buffer/vulkan_buffer.cpp
		command/vulkan_command_buffer.cpp
		device/vulkan_queue.cpp
		device/vulkan_physical_device.cpp
		pipeline/vulkan_pipeline.cpp
		pipeline/vulkan_pipeline_layout.cpp
		pipeline/vulkan_pipeline_cache.cpp
		vulkan_instance.cpp
		vulkan_sub_pass.cpp
		vulkan_semaphore.cpp
		vulkan_fence.cpp
		vulkan_shader.cpp
)
