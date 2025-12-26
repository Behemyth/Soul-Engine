/**
 * @file frame_allocator.ixx
 * @brief Per-frame GPU memory allocator for bindless root arguments
 * 
 * Provides a simple bump allocator pattern for per-frame GPU allocations.
 * All allocations are automatically freed when Reset() is called at frame end.
 * 
 * This follows Sebastian Aaltonen's "No Graphics API" pattern where per-draw
 * data (transforms, materials) is allocated fresh each frame.
 * 
 * @see https://www.sebastianaaltonen.com/blog/no-graphics-api
 */
export module synodic.soul.raster:frame_allocator;

import std;
import :gpu_pointer;

/**
 * @brief Abstract interface for per-frame GPU allocations
 * 
 * Concrete implementations (VulkanFrameAllocator) handle the actual
 * GPU memory allocation via buffer device addresses.
 */
export class FrameAllocator {
public:
	virtual ~FrameAllocator() = default;
	
	/**
	 * @brief Allocate GPU memory with CPU mapping
	 * 
	 * Returns a GPU pointer that can be written to via CPU and
	 * passed to shaders via push constants.
	 * 
	 * @param sizeBytes Size in bytes to allocate
	 * @param alignment Alignment requirement (default 16 for GPU)
	 * @return GPUPointerRaw containing CPU and GPU addresses
	 */
	[[nodiscard]] virtual GPUPointerRaw Allocate(std::size_t sizeBytes, 
		std::size_t alignment = 16) = 0;
	
	/**
	 * @brief Allocate typed GPU memory
	 * 
	 * Convenience wrapper that allocates sizeof(T) bytes with proper alignment.
	 */
	template<GPUTransferable T>
	[[nodiscard]] GPUPointerRaw AllocateTyped() {
		return Allocate(sizeof(T), alignof(T));
	}
	
	/**
	 * @brief Allocate array of typed GPU memory
	 */
	template<GPUTransferable T>
	[[nodiscard]] GPUPointerRaw AllocateArray(std::size_t count) {
		return Allocate(sizeof(T) * count, alignof(T));
	}
	
	/**
	 * @brief Reset allocator for new frame
	 * 
	 * Must be called at the start of each frame before any allocations.
	 * This does not free memory, just resets the bump pointer.
	 */
	virtual void Reset() = 0;
	
	/**
	 * @brief Get current allocation offset (for debugging)
	 */
	[[nodiscard]] virtual std::size_t CurrentOffset() const noexcept = 0;
	
	/**
	 * @brief Get total buffer size
	 */
	[[nodiscard]] virtual std::size_t Capacity() const noexcept = 0;
};
