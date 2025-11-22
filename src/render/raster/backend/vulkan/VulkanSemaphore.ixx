
export module synodic.soul.raster.backend.vulkan:semaphore;
import std;
import vulkan_hpp;

export class VulkanSemaphore
{

public:

	VulkanSemaphore(vk::Device);
	~VulkanSemaphore() = default;

	VulkanSemaphore(const VulkanSemaphore&) = delete;
	VulkanSemaphore(VulkanSemaphore&&) noexcept = default;

	VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;
	VulkanSemaphore& operator=(VulkanSemaphore&&) noexcept = default;

	[[nodiscard]] vk::Semaphore Handle() const;
	
private:
	
	vk::Device device_;
	
	vk::Semaphore semaphore_;

};
