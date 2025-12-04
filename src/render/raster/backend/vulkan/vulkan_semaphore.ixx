
export module synodic.soul.raster.backend.vulkan:semaphore;
import std;
import vulkan_hpp;

// Binary semaphore for swapchain operations (acquire/present)
export class VulkanSemaphore
{

public:

	VulkanSemaphore(vk::Device);

	~VulkanSemaphore() {
		if (semaphore_) {
			device_.destroySemaphore(semaphore_);
		}
	}

	VulkanSemaphore(const VulkanSemaphore&) = delete;

	VulkanSemaphore(VulkanSemaphore&& other) noexcept :
		device_(other.device_),
		semaphore_(other.semaphore_)
	{
		other.semaphore_ = nullptr;
		other.device_ = nullptr;
	}

	VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;

	VulkanSemaphore& operator=(VulkanSemaphore&& other) noexcept {
		if (this != &other) {
			if (semaphore_) {
				device_.destroySemaphore(semaphore_);
			}
			device_ = other.device_;
			semaphore_ = other.semaphore_;
			other.semaphore_ = nullptr;
			other.device_ = nullptr;
		}
		return *this;
	}

	[[nodiscard]] vk::Semaphore Handle() const;

private:

	vk::Device device_;

	vk::Semaphore semaphore_;

};

// Timeline semaphore for frame synchronization
export class VulkanTimelineSemaphore
{

public:

	VulkanTimelineSemaphore(vk::Device device, std::uint64_t initialValue = 0);

	~VulkanTimelineSemaphore() {
		if (semaphore_) {
			device_.destroySemaphore(semaphore_);
		}
	}

	VulkanTimelineSemaphore(const VulkanTimelineSemaphore&) = delete;

	VulkanTimelineSemaphore(VulkanTimelineSemaphore&& other) noexcept :
		device_(other.device_),
		semaphore_(other.semaphore_)
	{
		other.semaphore_ = nullptr;
		other.device_ = nullptr;
	}

	VulkanTimelineSemaphore& operator=(const VulkanTimelineSemaphore&) = delete;

	VulkanTimelineSemaphore& operator=(VulkanTimelineSemaphore&& other) noexcept {
		if (this != &other) {
			if (semaphore_) {
				device_.destroySemaphore(semaphore_);
			}
			device_ = other.device_;
			semaphore_ = other.semaphore_;
			other.semaphore_ = nullptr;
			other.device_ = nullptr;
		}
		return *this;
	}

	[[nodiscard]] vk::Semaphore Handle() const;

	// Wait on CPU for semaphore to reach at least the given value
	void Wait(std::uint64_t value) const;

	// Signal from CPU to the given value
	void Signal(std::uint64_t value) const;

	// Get the current counter value
	[[nodiscard]] std::uint64_t GetValue() const;

private:

	vk::Device device_;
	vk::Semaphore semaphore_;

};
