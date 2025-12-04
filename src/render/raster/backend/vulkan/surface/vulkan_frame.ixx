export module synodic.soul.raster.backend.vulkan:frame;

import std;

import :framebuffer;
import :semaphore;
import :fence;
import synodic.soul.scheduler;

export template<SchedulerBackend SchedulerType>
class VulkanFrame{

public:

	VulkanFrame() = default;
	~VulkanFrame() = default;

	VulkanFrame(const VulkanFrame&) = delete;
	VulkanFrame(VulkanFrame&&) noexcept = default;

	VulkanFrame& operator=(const VulkanFrame&) = delete;
	VulkanFrame& operator=(VulkanFrame&&) noexcept = default;

	VulkanFrameBuffer<SchedulerType>& Framebuffer() {
		return framebuffer_.value();
	}

	VulkanSemaphore& RenderSemaphore() {
		return renderSemaphore_.value();
	}

private:
	
	std::optional<VulkanFrameBuffer<SchedulerType>> framebuffer_;
	std::optional<VulkanSemaphore> presentSemaphore_;
	std::optional<VulkanSemaphore> renderSemaphore_;
	std::optional<VulkanFence> imageFence_;
	
};
