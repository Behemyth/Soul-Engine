module synodic.soul.raster.backend.vulkan;

VulkanBuffer::VulkanBuffer(VulkanAllocator& allocator,
	vk::DeviceSize size,
	vk::BufferUsageFlags usage,
	const VulkanAllocationCreateInfo& allocInfo)
	: allocator_(&allocator)
	, buffer_(nullptr)
	, allocation_(nullptr)
	, size_(size)
	, mappedData_(nullptr)
	, isPersistentlyMapped_(allocInfo.mapped)
{
	auto result = allocator_->CreateBuffer(size, usage, allocInfo);

	if (!result) {
		throw std::runtime_error(ToString(result.error()));
	}

	auto allocation = result.value();
	buffer_ = allocation.buffer;
	allocation_ = allocation.allocation;
	mappedData_ = allocation.mappedData;
}

VulkanBuffer::~VulkanBuffer()
{
	if (buffer_ && allocation_) {
		allocator_->DestroyBuffer(buffer_, allocation_);
	}
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
	: allocator_(other.allocator_)
	, buffer_(other.buffer_)
	, allocation_(other.allocation_)
	, size_(other.size_)
	, mappedData_(other.mappedData_)
	, isPersistentlyMapped_(other.isPersistentlyMapped_)
{
	other.buffer_ = nullptr;
	other.allocation_ = nullptr;
	other.mappedData_ = nullptr;
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
{
	if (this != &other) {
		if (buffer_ && allocation_) {
			allocator_->DestroyBuffer(buffer_, allocation_);
		}

		allocator_ = other.allocator_;
		buffer_ = other.buffer_;
		allocation_ = other.allocation_;
		size_ = other.size_;
		mappedData_ = other.mappedData_;
		isPersistentlyMapped_ = other.isPersistentlyMapped_;

		other.buffer_ = nullptr;
		other.allocation_ = nullptr;
		other.mappedData_ = nullptr;
	}
	return *this;
}

VulkanResult<std::span<std::byte>> VulkanBuffer::Map()
{
	if (isPersistentlyMapped_ && mappedData_) {
		return std::span<std::byte>(static_cast<std::byte*>(mappedData_), size_);
	}

	if (mappedData_) {
		// Already mapped
		return std::span<std::byte>(static_cast<std::byte*>(mappedData_), size_);
	}

	auto result = allocator_->MapMemory(allocation_);
	if (!result) {
		return std::unexpected(result.error());
	}

	mappedData_ = result.value();
	return std::span<std::byte>(static_cast<std::byte*>(mappedData_), size_);
}

void VulkanBuffer::Unmap()
{
	if (isPersistentlyMapped_) {
		// Don't unmap persistently mapped memory
		return;
	}

	if (mappedData_) {
		allocator_->UnmapMemory(allocation_);
		mappedData_ = nullptr;
	}
}

VulkanVoidResult VulkanBuffer::Flush(vk::DeviceSize offset, vk::DeviceSize size)
{
	vk::DeviceSize flushSize = (size == vk::WholeSize) ? size_ - offset : size;
	return allocator_->FlushAllocation(allocation_, offset, flushSize);
}

VulkanVoidResult VulkanBuffer::Invalidate(vk::DeviceSize offset, vk::DeviceSize size)
{
	vk::DeviceSize invalidateSize = (size == vk::WholeSize) ? size_ - offset : size;
	return allocator_->InvalidateAllocation(allocation_, offset, invalidateSize);
}
