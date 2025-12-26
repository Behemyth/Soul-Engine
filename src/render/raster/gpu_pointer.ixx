/**
 * @file gpu_pointer.ixx
 * @brief Bindless GPU pointer abstraction for modern graphics APIs
 * 
 * Implements Sebastian Aaltonen's "No Graphics API" pattern where shader data
 * is passed via 64-bit GPU pointers instead of descriptor sets or push constants.
 * 
 * This enables:
 * - Direct CPU writes to GPU memory (ReBAR/UMA systems)
 * - GPU-generated shader arguments for indirect rendering
 * - Unified CPU/GPU struct headers (future: reflection-based codegen)
 * 
 * @see https://www.sebastianaaltonen.com/blog/no-graphics-api
 */
export module synodic.soul.raster:gpu_pointer;

import std;

/**
 * @brief Concept ensuring types are safe for GPU transfer
 * 
 * GPU memory transfers require trivially copyable types to ensure
 * bit-exact representation between CPU and GPU memory.
 */
export template<typename T>
concept GPUTransferable = std::is_trivially_copyable_v<T>;

/**
 * @brief 64-bit GPU device address
 * 
 * Represents a raw GPU virtual address obtained from VK_KHR_buffer_device_address.
 * Can be passed directly to shaders via root arguments.
 */
export using GPUDeviceAddress = std::uint64_t;

/**
 * @brief Invalid GPU address sentinel
 */
export constexpr GPUDeviceAddress InvalidGPUAddress = 0;

/**
 * @brief Raw (untyped) GPU pointer pair
 * 
 * Used by allocators to return CPU/GPU address pairs before casting to typed.
 */
export struct GPUPointerRaw {
	void* cpu = nullptr;
	GPUDeviceAddress gpu = InvalidGPUAddress;
	
	[[nodiscard]] constexpr bool IsValid() const noexcept {
		return cpu != nullptr && gpu != InvalidGPUAddress;
	}
	
	/**
	 * @brief Cast to typed pointer
	 */
	template<GPUTransferable T>
	[[nodiscard]] T* As() const noexcept {
		return static_cast<T*>(cpu);
	}
};

/**
 * @brief Bindless GPU pointer with paired CPU/GPU addresses
 * 
 * Wraps a GPU allocation providing both CPU-mapped pointer for direct writes
 * and GPU device address for shader access. Follows the CUDA-style memory model
 * where CPU can write directly to GPU memory via ReBAR/PCIe BAR.
 * 
 * @tparam T The pointed-to type, must satisfy GPUTransferable concept
 * 
 * Usage:
 * @code
 * struct ShaderData {
 *     float4 color;
 *     const Vertex* vertices;  // GPU pointer
 *     uint32_t vertexCount;
 * };
 * 
 * GPUPointer<ShaderData> data = allocator.Allocate<ShaderData>();
 * data.CPU()->color = {1, 0, 0, 1};
 * data.CPU()->vertices = vertexBuffer.GPU();
 * 
 * commandList.Dispatch(data.GPU(), {128, 1, 1});
 * @endcode
 */
export template<GPUTransferable T>
class GPUPointer {
public:
	/**
	 * @brief Default constructor - creates null pointer
	 */
	constexpr GPUPointer() noexcept = default;
	
	/**
	 * @brief Construct from CPU and GPU address pair
	 * 
	 * @param cpuPtr CPU-mapped pointer for host writes
	 * @param gpuAddress GPU device address for shader access
	 */
	constexpr GPUPointer(T* cpuPtr, GPUDeviceAddress gpuAddress) noexcept
		: cpu_(cpuPtr)
		, gpu_(gpuAddress)
	{}
	
	~GPUPointer() = default;
	
	// Non-copyable to prevent accidental shallow copies of GPU resources
	GPUPointer(const GPUPointer&) = delete;
	GPUPointer& operator=(const GPUPointer&) = delete;
	
	// Move-only semantics
	constexpr GPUPointer(GPUPointer&& other) noexcept
		: cpu_(other.cpu_)
		, gpu_(other.gpu_)
	{
		other.cpu_ = nullptr;
		other.gpu_ = InvalidGPUAddress;
	}
	
	constexpr GPUPointer& operator=(GPUPointer&& other) noexcept {
		if (this != &other) {
			cpu_ = other.cpu_;
			gpu_ = other.gpu_;
			other.cpu_ = nullptr;
			other.gpu_ = InvalidGPUAddress;
		}
		return *this;
	}
	
	/**
	 * @brief Get CPU-mapped pointer for direct host writes
	 * @return Pointer to CPU-accessible memory
	 */
	[[nodiscard]] constexpr T* CPU() noexcept { return cpu_; }
	
	/**
	 * @brief Get CPU-mapped pointer (const)
	 * @return Const pointer to CPU-accessible memory
	 */
	[[nodiscard]] constexpr const T* CPU() const noexcept { return cpu_; }
	
	/**
	 * @brief Get GPU device address for shader access
	 * @return 64-bit GPU virtual address
	 */
	[[nodiscard]] constexpr GPUDeviceAddress GPU() const noexcept { return gpu_; }
	
	/**
	 * @brief Check if pointer is valid (non-null)
	 * @return true if both CPU and GPU addresses are valid
	 */
	[[nodiscard]] constexpr bool IsValid() const noexcept {
		return cpu_ != nullptr && gpu_ != InvalidGPUAddress;
	}
	
	/**
	 * @brief Explicit bool conversion for validity checks
	 */
	[[nodiscard]] constexpr explicit operator bool() const noexcept {
		return IsValid();
	}
	
	/**
	 * @brief Dereference operator for CPU access
	 * @return Reference to the pointed-to value
	 */
	[[nodiscard]] constexpr T& operator*() noexcept { return *cpu_; }
	
	/**
	 * @brief Dereference operator for CPU access (const)
	 * @return Const reference to the pointed-to value
	 */
	[[nodiscard]] constexpr const T& operator*() const noexcept { return *cpu_; }
	
	/**
	 * @brief Member access operator for CPU access
	 * @return Pointer to the pointed-to value
	 */
	[[nodiscard]] constexpr T* operator->() noexcept { return cpu_; }
	
	/**
	 * @brief Member access operator for CPU access (const)
	 * @return Const pointer to the pointed-to value
	 */
	[[nodiscard]] constexpr const T* operator->() const noexcept { return cpu_; }
	
	/**
	 * @brief Calculate GPU address with byte offset
	 * 
	 * Useful for accessing array elements or struct members via GPU address.
	 * 
	 * @param byteOffset Offset in bytes from base address
	 * @return Offset GPU address
	 */
	[[nodiscard]] constexpr GPUDeviceAddress GPUOffset(std::size_t byteOffset) const noexcept {
		return gpu_ + byteOffset;
	}
	
	/**
	 * @brief Calculate GPU address for array element
	 * 
	 * @param index Array index
	 * @return GPU address of element at index
	 */
	[[nodiscard]] constexpr GPUDeviceAddress GPUElement(std::size_t index) const noexcept {
		return gpu_ + (index * sizeof(T));
	}
	
	/**
	 * @brief Release ownership and return raw pointers
	 * 
	 * Transfers ownership to caller. After this call, the GPUPointer is null.
	 * 
	 * @return Pair of {CPU pointer, GPU address}
	 */
	[[nodiscard]] constexpr std::pair<T*, GPUDeviceAddress> Release() noexcept {
		auto result = std::make_pair(cpu_, gpu_);
		cpu_ = nullptr;
		gpu_ = InvalidGPUAddress;
		return result;
	}
	
	/**
	 * @brief Reset to null state
	 * 
	 * Does NOT free GPU memory - that is the allocator's responsibility.
	 */
	constexpr void Reset() noexcept {
		cpu_ = nullptr;
		gpu_ = InvalidGPUAddress;
	}

private:
	T* cpu_ = nullptr;
	GPUDeviceAddress gpu_ = InvalidGPUAddress;
};

/**
 * @brief Type-erased GPU pointer for command recording
 * 
 * Used when the command list needs to store GPU pointers without
 * knowing the concrete type (e.g., for generic dispatch/draw commands).
 */
export struct GPUPointerUntyped {
	void* cpu = nullptr;
	GPUDeviceAddress gpu = InvalidGPUAddress;
	std::size_t size = 0;  // Size in bytes for validation
	
	constexpr GPUPointerUntyped() noexcept = default;
	
	constexpr GPUPointerUntyped(void* cpuPtr, GPUDeviceAddress gpuAddr, std::size_t sizeBytes) noexcept
		: cpu(cpuPtr)
		, gpu(gpuAddr)
		, size(sizeBytes)
	{}
	
	/**
	 * @brief Construct from typed GPUPointer
	 */
	template<GPUTransferable T>
	constexpr GPUPointerUntyped(const GPUPointer<T>& ptr) noexcept
		: cpu(const_cast<T*>(ptr.CPU()))
		, gpu(ptr.GPU())
		, size(sizeof(T))
	{}
	
	[[nodiscard]] constexpr bool IsValid() const noexcept {
		return cpu != nullptr && gpu != InvalidGPUAddress;
	}
	
	[[nodiscard]] constexpr explicit operator bool() const noexcept {
		return IsValid();
	}
};

/**
 * @brief GPU pointer to array with element count
 * 
 * Combines GPU pointer semantics with array bounds for safer buffer access.
 * 
 * @tparam T Element type, must satisfy GPUTransferable
 */
export template<GPUTransferable T>
class GPUSpan {
public:
	constexpr GPUSpan() noexcept = default;
	
	constexpr GPUSpan(T* cpuPtr, GPUDeviceAddress gpuAddress, std::size_t count) noexcept
		: cpu_(cpuPtr)
		, gpu_(gpuAddress)
		, count_(count)
	{}
	
	~GPUSpan() = default;
	
	GPUSpan(const GPUSpan&) = delete;
	GPUSpan& operator=(const GPUSpan&) = delete;
	
	constexpr GPUSpan(GPUSpan&& other) noexcept
		: cpu_(other.cpu_)
		, gpu_(other.gpu_)
		, count_(other.count_)
	{
		other.cpu_ = nullptr;
		other.gpu_ = InvalidGPUAddress;
		other.count_ = 0;
	}
	
	constexpr GPUSpan& operator=(GPUSpan&& other) noexcept {
		if (this != &other) {
			cpu_ = other.cpu_;
			gpu_ = other.gpu_;
			count_ = other.count_;
			other.cpu_ = nullptr;
			other.gpu_ = InvalidGPUAddress;
			other.count_ = 0;
		}
		return *this;
	}
	
	[[nodiscard]] constexpr T* CPU() noexcept { return cpu_; }
	[[nodiscard]] constexpr const T* CPU() const noexcept { return cpu_; }
	[[nodiscard]] constexpr GPUDeviceAddress GPU() const noexcept { return gpu_; }
	[[nodiscard]] constexpr std::size_t Count() const noexcept { return count_; }
	[[nodiscard]] constexpr std::size_t SizeBytes() const noexcept { return count_ * sizeof(T); }
	[[nodiscard]] constexpr bool Empty() const noexcept { return count_ == 0; }
	
	[[nodiscard]] constexpr bool IsValid() const noexcept {
		return cpu_ != nullptr && gpu_ != InvalidGPUAddress;
	}
	
	[[nodiscard]] constexpr explicit operator bool() const noexcept {
		return IsValid();
	}
	
	/**
	 * @brief Array subscript for CPU access
	 */
	[[nodiscard]] constexpr T& operator[](std::size_t index) noexcept {
		return cpu_[index];
	}
	
	[[nodiscard]] constexpr const T& operator[](std::size_t index) const noexcept {
		return cpu_[index];
	}
	
	/**
	 * @brief Get GPU address of element at index
	 */
	[[nodiscard]] constexpr GPUDeviceAddress GPUElement(std::size_t index) const noexcept {
		return gpu_ + (index * sizeof(T));
	}
	
	/**
	 * @brief Get std::span view of CPU memory
	 */
	[[nodiscard]] constexpr std::span<T> CPUSpan() noexcept {
		return std::span<T>(cpu_, count_);
	}
	
	[[nodiscard]] constexpr std::span<const T> CPUSpan() const noexcept {
		return std::span<const T>(cpu_, count_);
	}

private:
	T* cpu_ = nullptr;
	GPUDeviceAddress gpu_ = InvalidGPUAddress;
	std::size_t count_ = 0;
};

/**
 * @brief Type-erased GPU span for command recording
 */
export struct GPUSpanUntyped {
	void* cpu = nullptr;
	GPUDeviceAddress gpu = InvalidGPUAddress;
	std::size_t count = 0;
	std::size_t stride = 0;  // Size of each element
	
	constexpr GPUSpanUntyped() noexcept = default;
	
	constexpr GPUSpanUntyped(void* cpuPtr, GPUDeviceAddress gpuAddr, 
	                         std::size_t elementCount, std::size_t elementStride) noexcept
		: cpu(cpuPtr)
		, gpu(gpuAddr)
		, count(elementCount)
		, stride(elementStride)
	{}
	
	template<GPUTransferable T>
	constexpr GPUSpanUntyped(const GPUSpan<T>& span) noexcept
		: cpu(const_cast<T*>(span.CPU()))
		, gpu(span.GPU())
		, count(span.Count())
		, stride(sizeof(T))
	{}
	
	[[nodiscard]] constexpr std::size_t SizeBytes() const noexcept {
		return count * stride;
	}
	
	[[nodiscard]] constexpr bool IsValid() const noexcept {
		return cpu != nullptr && gpu != InvalidGPUAddress;
	}
};

