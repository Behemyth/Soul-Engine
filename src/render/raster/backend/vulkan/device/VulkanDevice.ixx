export module render.raster.vulkan:device;

import std;
import vulkan_hpp;

import :queue;
import :allocator;
import :error;
import synodic.soul.scheduler;
import synodic.soul.engine;

// TODO: VulkanDevice should inherit from RasterDevice but this causes
// MSVC Internal Compiler Error when importing synodic.soul.raster
// For now, VulkanDevice just implements the same interface
export class VulkanDevice final {

public:

	VulkanDevice(std::shared_ptr<SchedulerModule>&,
		const vk::Instance&,
		const vk::PhysicalDevice&,
		std::span<std::string>,
		std::span<std::string>,
		std::uint32_t vulkanApiVersion);
	~VulkanDevice();

	VulkanDevice(const VulkanDevice &) = delete;
	VulkanDevice(VulkanDevice &&) noexcept = default;

	VulkanDevice& operator=(const VulkanDevice &) = delete;
	VulkanDevice& operator=(VulkanDevice&&) noexcept = default;

	void Synchronize();

	const vk::Device& Logical() const;
	const vk::PhysicalDevice& Physical() const;
	VulkanAllocator& Allocator() noexcept { return allocator_; }
	const VulkanAllocator& Allocator() const noexcept { return allocator_; }

	bool SurfaceSupported(vk::SurfaceKHR&);
	VulkanResult<std::uint32_t> HighFamilyIndex() const;
	std::span<VulkanQueue> GraphicsQueues();
	std::span<VulkanQueue> ComputeQueues();
	std::span<VulkanQueue> TransferQueues();

private:

	std::shared_ptr<SchedulerModule> scheduler_;

	vk::Device device_;
	vk::PhysicalDevice physicalDevice_;

	std::vector<VulkanQueue> graphicsQueues_;
	std::vector<VulkanQueue> computeQueues_;
	std::vector<VulkanQueue> transferQueues_;

	VulkanAllocator allocator_;

};
