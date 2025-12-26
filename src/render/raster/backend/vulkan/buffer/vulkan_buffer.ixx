export module synodic.soul.raster.backend.vulkan:buffer;

import :device;
import :allocator;
import :error;

import std;
import vulkan;

// Non-templated base for type-erased buffer management
export class VulkanBuffer {

public:

	VulkanBuffer(VulkanAllocator& allocator,
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		const VulkanAllocationCreateInfo& allocInfo);
	virtual ~VulkanBuffer();

	VulkanBuffer(const VulkanBuffer&) = delete;
	VulkanBuffer(VulkanBuffer&& other) noexcept;

	VulkanBuffer& operator=(const VulkanBuffer&) = delete;
	VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;

	vk::Buffer Handle() const noexcept { return buffer_; }
	vk::DeviceSize Size() const noexcept { return size_; }

	// Map memory - returns span of bytes for type-safe access
	VulkanResult<std::span<std::byte>> Map();
	void Unmap();

	// Typed access helpers
	template<typename T>
	VulkanResult<std::span<T>> MapAs() {
		auto result = Map();
		if (!result) {
			return std::unexpected(result.error());
		}
		auto bytes = result.value();
		return std::span<T>(reinterpret_cast<T*>(bytes.data()), bytes.size() / sizeof(T));
	}

	// Flush/invalidate for non-coherent memory
	VulkanVoidResult Flush(vk::DeviceSize offset = 0, vk::DeviceSize size = vk::WholeSize);
	VulkanVoidResult Invalidate(vk::DeviceSize offset = 0, vk::DeviceSize size = vk::WholeSize);

private:
	VulkanAllocator* allocator_;
	vk::Buffer buffer_;
	VmaAllocationHandle allocation_;
	vk::DeviceSize size_;
	void* mappedData_;
	bool isPersistentlyMapped_;

};

// Implementation removed - now in VulkanBuffer.cpp
