
export module synodic.soul.raster.backend.vulkan:fence;
import std;
import vulkan;

export class VulkanFence
{

public:

	VulkanFence(vk::Device);

	~VulkanFence() {
		if (fence_) {
			device_.destroyFence(fence_);
		}
	}

	VulkanFence(const VulkanFence&) = delete;

	VulkanFence(VulkanFence&& other) noexcept :
		device_(other.device_),
		fence_(other.fence_)
	{
		other.fence_ = nullptr;
		other.device_ = nullptr;
	}

	VulkanFence& operator=(const VulkanFence&) = delete;

	VulkanFence& operator=(VulkanFence&& other) noexcept {
		if (this != &other) {
			if (fence_) {
				device_.destroyFence(fence_);
			}
			device_ = other.device_;
			fence_ = other.fence_;
			other.fence_ = nullptr;
			other.device_ = nullptr;
		}
		return *this;
	}

	[[nodiscard]] vk::Fence Handle() const {
		return fence_;
	}

	void Wait() const {
		if (fence_) {
			static_cast<void>(device_.waitForFences(fence_, vk::True, std::numeric_limits<std::uint64_t>::max()));
		}
	}

	void Reset() const {
		if (fence_) {
			device_.resetFences(fence_);
		}
	}


private:

	vk::Device device_;

	vk::Fence fence_;

};
