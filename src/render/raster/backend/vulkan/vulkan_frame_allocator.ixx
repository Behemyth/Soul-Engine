/**
 * @file vulkan_frame_allocator.ixx
 * @brief Vulkan per-frame GPU memory allocator
 * 
 * Implements a simple ring-buffer allocator using a large persistently-mapped
 * buffer with buffer device address. Each frame gets a section of the buffer,
 * allowing lock-free allocation during rendering.
 */
export module synodic.soul.raster.backend.vulkan:frame_allocator;

import std;
import vulkan;

import synodic.soul.raster;
import :allocator;

/**
 * @brief Per-frame section of the ring buffer
 */
struct FrameSection {
	std::size_t offset = 0;     // Start offset in main buffer
	std::size_t size = 0;       // Max size for this frame
	std::size_t current = 0;    // Current bump pointer within section
};

/**
 * @brief Vulkan frame allocator using ring buffer pattern
 * 
 * Allocates from a large persistently-mapped buffer with buffer device address.
 * The buffer is divided into N sections (one per frame in flight), cycling
 * through them to avoid CPU-GPU synchronization on allocation.
 */
export class VulkanFrameAllocator : public FrameAllocator {
public:
	static constexpr std::size_t DefaultBufferSize = 16 * 1024 * 1024;  // 16 MB per frame
	static constexpr std::uint32_t FrameCount = 3;  // Triple buffering
	
	/**
	 * @brief Create frame allocator
	 * 
	 * @param device Vulkan logical device
	 * @param allocator VMA allocator
	 * @param sizePerFrame Size in bytes for each frame's allocation pool
	 */
	VulkanFrameAllocator(
		vk::Device device,
		VulkanAllocator& allocator,
		std::size_t sizePerFrame = DefaultBufferSize);
	
	~VulkanFrameAllocator();
	
	VulkanFrameAllocator(const VulkanFrameAllocator&) = delete;
	VulkanFrameAllocator& operator=(const VulkanFrameAllocator&) = delete;
	
	VulkanFrameAllocator(VulkanFrameAllocator&& other) noexcept;
	VulkanFrameAllocator& operator=(VulkanFrameAllocator&& other) noexcept;
	
	/**
	 * @brief Allocate from current frame's section
	 */
	[[nodiscard]] GPUPointerRaw Allocate(std::size_t sizeBytes, 
		std::size_t alignment = 16) override;
	
	/**
	 * @brief Advance to next frame (call at frame start)
	 */
	void Reset() override;
	
	[[nodiscard]] std::size_t CurrentOffset() const noexcept override {
		return frames_[currentFrame_].current;
	}
	
	[[nodiscard]] std::size_t Capacity() const noexcept override {
		return sizePerFrame_;
	}
	
	/**
	 * @brief Get current frame index
	 */
	[[nodiscard]] std::uint32_t CurrentFrameIndex() const noexcept {
		return currentFrame_;
	}

private:
	vk::Device device_ = nullptr;
	VulkanAllocator* allocator_ = nullptr;
	
	// Ring buffer
	vk::Buffer buffer_ = nullptr;
	VmaAllocationHandle allocation_ = nullptr;
	void* mappedPtr_ = nullptr;
	GPUDeviceAddress gpuBaseAddress_ = InvalidGPUAddress;
	
	std::size_t sizePerFrame_ = 0;
	std::uint32_t currentFrame_ = 0;
	std::array<FrameSection, FrameCount> frames_;
};

// Implementation

VulkanFrameAllocator::VulkanFrameAllocator(
	vk::Device device,
	VulkanAllocator& allocator,
	std::size_t sizePerFrame)
	: device_(device)
	, allocator_(&allocator)
	, sizePerFrame_(sizePerFrame)
{
	std::size_t totalSize = sizePerFrame * FrameCount;
	
	// Buffer usage flags (storage + device address)
	vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eStorageBuffer |
	                             vk::BufferUsageFlagBits::eShaderDeviceAddress;
	
	// Allocate with host-visible + coherent memory for direct CPU writes
	VulkanAllocationCreateInfo allocInfo;
	allocInfo.usage = MemoryUsage::CpuToGpu;
	allocInfo.mapped = true;  // Persistently mapped
	
	auto result = allocator_->CreateBuffer(totalSize, usage, allocInfo);
	
	if (!result) {
		throw std::runtime_error("VulkanFrameAllocator: Failed to create ring buffer");
	}
	
	buffer_ = result->buffer;
	allocation_ = result->allocation;
	mappedPtr_ = result->mappedData;
	
	// Get GPU device address
	vk::BufferDeviceAddressInfo addressInfo;
	addressInfo.buffer = buffer_;
	gpuBaseAddress_ = device_.getBufferAddress(addressInfo);
	
	// Initialize frame sections
	for (std::uint32_t i = 0; i < FrameCount; ++i) {
		frames_[i].offset = i * sizePerFrame;
		frames_[i].size = sizePerFrame;
		frames_[i].current = 0;
	}
}

VulkanFrameAllocator::~VulkanFrameAllocator() {
	if (buffer_ && allocator_) {
		allocator_->DestroyBuffer(buffer_, allocation_);
	}
}

VulkanFrameAllocator::VulkanFrameAllocator(VulkanFrameAllocator&& other) noexcept
	: device_(other.device_)
	, allocator_(other.allocator_)
	, buffer_(other.buffer_)
	, allocation_(other.allocation_)
	, mappedPtr_(other.mappedPtr_)
	, gpuBaseAddress_(other.gpuBaseAddress_)
	, sizePerFrame_(other.sizePerFrame_)
	, currentFrame_(other.currentFrame_)
	, frames_(other.frames_)
{
	other.buffer_ = nullptr;
	other.allocation_ = nullptr;
	other.mappedPtr_ = nullptr;
	other.gpuBaseAddress_ = InvalidGPUAddress;
}

VulkanFrameAllocator& VulkanFrameAllocator::operator=(VulkanFrameAllocator&& other) noexcept {
	if (this != &other) {
		if (buffer_ && allocator_) {
			allocator_->DestroyBuffer(buffer_, allocation_);
		}
		
		device_ = other.device_;
		allocator_ = other.allocator_;
		buffer_ = other.buffer_;
		allocation_ = other.allocation_;
		mappedPtr_ = other.mappedPtr_;
		gpuBaseAddress_ = other.gpuBaseAddress_;
		sizePerFrame_ = other.sizePerFrame_;
		currentFrame_ = other.currentFrame_;
		frames_ = other.frames_;
		
		other.buffer_ = nullptr;
		other.allocation_ = nullptr;
		other.mappedPtr_ = nullptr;
		other.gpuBaseAddress_ = InvalidGPUAddress;
	}
	return *this;
}

GPUPointerRaw VulkanFrameAllocator::Allocate(std::size_t sizeBytes, std::size_t alignment) {
	auto& frame = frames_[currentFrame_];
	
	// Align the current offset
	std::size_t alignedOffset = (frame.current + alignment - 1) & ~(alignment - 1);
	
	if (alignedOffset + sizeBytes > frame.size) {
		// Out of memory for this frame
		return GPUPointerRaw{};
	}
	
	std::size_t bufferOffset = frame.offset + alignedOffset;
	frame.current = alignedOffset + sizeBytes;
	
	return GPUPointerRaw{
		.cpu = static_cast<std::byte*>(mappedPtr_) + bufferOffset,
		.gpu = gpuBaseAddress_ + bufferOffset
	};
}

void VulkanFrameAllocator::Reset() {
	// Move to next frame
	currentFrame_ = (currentFrame_ + 1) % FrameCount;
	
	// Reset that frame's bump pointer
	frames_[currentFrame_].current = 0;
}
