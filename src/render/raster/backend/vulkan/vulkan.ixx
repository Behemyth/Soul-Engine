export module synodic.soul.raster.backend.vulkan;

export import :dispatch;  // Dynamic dispatcher initialization (must be first)
export import :error;
export import :allocator;
export import :physical_device;
export import :queue;
export import :device;
export import :instance;
export import :buffer;
export import :command_pool;
export import :command_buffer;
export import :fence;
export import :semaphore;
export import :framebuffer;
export import :render_pass;
export import :shader;
export import :subpass;
export import :pipeline_layout;
export import :pipeline_cache;
export import :pipeline;
export import :surface;
export import :swapchain;
export import :frame;
export import :backend;  // VulkanRasterBackend class
// Bindless resources (No Graphics API pattern)
export import :texture_heap;
export import :sampler_heap;
export import :gpu_allocator;
export import :bindless_layout;
export import :frame_allocator;
