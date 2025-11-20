module;

#include <vulkan/vulkan.hpp>

export module render.raster.vulkan:device;

import synodic.soul.engine;
import :queue;

using std::uint32_t = std::uint32_t;

export class VulkanDevice final: public RasterDevice {

public:

	VulkanDevice(std::shared_ptr<SchedulerModule>&,
		const vk::Instance&,
		const vk::PhysicalDevice&,
		nonstd::span<std::string>,
		nonstd::span<std::string>);
	~VulkanDevice() override;

	VulkanDevice(const VulkanDevice &) = delete;
	VulkanDevice(VulkanDevice &&) noexcept = default;

	VulkanDevice& operator=(const VulkanDevice &) = delete;
	VulkanDevice& operator=(VulkanDevice&&) noexcept = default;

	void Synchronize() override;

	const vk::Device& Logical() const;
	const vk::PhysicalDevice& Physical() const;
	const vk::DispatchLoaderDynamic& DispatchLoader() const;

	bool SurfaceSupported(vk::SurfaceKHR&);
	std::uint32_t HighFamilyIndex() const;
	nonstd::span<VulkanQueue> GraphicsQueues();
	nonstd::span<VulkanQueue> ComputeQueues();
	nonstd::span<VulkanQueue> TransferQueues();

private:

	std::shared_ptr<SchedulerModule> scheduler_;

	vk::Device device_;
	vk::PhysicalDevice physicalDevice_;

	std::vector<VulkanQueue> graphicsQueues_;
	std::vector<VulkanQueue> computeQueues_;
	std::vector<VulkanQueue> transferQueues_;

	// Dynamic dispatcher for extensions
	vk::DispatchLoaderDynamic dispatcher_;


};
