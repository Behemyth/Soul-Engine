export module synodic.soul.raster.backend.vulkan:descriptor_pool;

import std;
import vulkan_hpp;

// Configuration for descriptor pool sizes
export struct DescriptorPoolSizes {
	std::uint32_t uniformBuffers = 100;
	std::uint32_t storageBuffers = 100;
	std::uint32_t sampledImages = 100;
	std::uint32_t samplers = 100;
	std::uint32_t combinedImageSamplers = 100;
	std::uint32_t storageImages = 50;
	std::uint32_t maxSets = 100;
	
	// Convenience presets
	static DescriptorPoolSizes Small() {
		return {20, 20, 20, 20, 20, 10, 50};
	}
	
	static DescriptorPoolSizes Default() {
		return {};  // Uses default values
	}
	
	static DescriptorPoolSizes Large() {
		return {500, 500, 500, 500, 500, 200, 1000};
	}
};

// Manages a pool of descriptor sets for efficient allocation
export class VulkanDescriptorPool {
public:
	VulkanDescriptorPool() = default;
	
	VulkanDescriptorPool(vk::Device device, const DescriptorPoolSizes& sizes)
		: device_(device)
		, maxSets_(sizes.maxSets)
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		
		if (sizes.uniformBuffers > 0) {
			poolSizes.push_back({vk::DescriptorType::eUniformBuffer, sizes.uniformBuffers});
		}
		if (sizes.storageBuffers > 0) {
			poolSizes.push_back({vk::DescriptorType::eStorageBuffer, sizes.storageBuffers});
		}
		if (sizes.sampledImages > 0) {
			poolSizes.push_back({vk::DescriptorType::eSampledImage, sizes.sampledImages});
		}
		if (sizes.samplers > 0) {
			poolSizes.push_back({vk::DescriptorType::eSampler, sizes.samplers});
		}
		if (sizes.combinedImageSamplers > 0) {
			poolSizes.push_back({vk::DescriptorType::eCombinedImageSampler, sizes.combinedImageSamplers});
		}
		if (sizes.storageImages > 0) {
			poolSizes.push_back({vk::DescriptorType::eStorageImage, sizes.storageImages});
		}
		
		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;  // Allow individual set freeing
		poolInfo.maxSets = sizes.maxSets;
		poolInfo.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		
		pool_ = device_.createDescriptorPool(poolInfo);
	}
	
	~VulkanDescriptorPool() {
		if (pool_ && device_) {
			device_.destroyDescriptorPool(pool_);
		}
	}
	
	VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
	VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;
	
	VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept
		: device_(other.device_)
		, pool_(other.pool_)
		, maxSets_(other.maxSets_)
		, allocatedSets_(other.allocatedSets_)
	{
		other.pool_ = nullptr;
		other.device_ = nullptr;
	}
	
	VulkanDescriptorPool& operator=(VulkanDescriptorPool&& other) noexcept {
		if (this != &other) {
			if (pool_ && device_) {
				device_.destroyDescriptorPool(pool_);
			}
			device_ = other.device_;
			pool_ = other.pool_;
			maxSets_ = other.maxSets_;
			allocatedSets_ = other.allocatedSets_;
			other.pool_ = nullptr;
			other.device_ = nullptr;
		}
		return *this;
	}
	
	// Allocate descriptor sets from this pool
	[[nodiscard]] std::vector<vk::DescriptorSet> Allocate(
		std::span<const vk::DescriptorSetLayout> layouts)
	{
		if (allocatedSets_ + layouts.size() > maxSets_) {
			return {};  // Pool exhausted
		}
		
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.descriptorPool = pool_;
		allocInfo.descriptorSetCount = static_cast<std::uint32_t>(layouts.size());
		allocInfo.pSetLayouts = layouts.data();
		
		auto sets = device_.allocateDescriptorSets(allocInfo);
		allocatedSets_ += static_cast<std::uint32_t>(sets.size());
		return sets;
	}
	
	// Allocate a single descriptor set
	[[nodiscard]] vk::DescriptorSet AllocateOne(vk::DescriptorSetLayout layout) {
		auto sets = Allocate(std::span(&layout, 1));
		return sets.empty() ? vk::DescriptorSet{} : sets[0];
	}
	
	// Free descriptor sets back to the pool
	void Free(std::span<const vk::DescriptorSet> sets) {
		if (!sets.empty()) {
			device_.freeDescriptorSets(pool_, sets);
			allocatedSets_ -= static_cast<std::uint32_t>(sets.size());
		}
	}
	
	// Reset the entire pool (frees all allocated sets)
	void Reset() {
		device_.resetDescriptorPool(pool_);
		allocatedSets_ = 0;
	}
	
	[[nodiscard]] vk::DescriptorPool Handle() const { return pool_; }
	[[nodiscard]] std::uint32_t AllocatedCount() const { return allocatedSets_; }
	[[nodiscard]] std::uint32_t RemainingCapacity() const { return maxSets_ - allocatedSets_; }

private:
	vk::Device device_;
	vk::DescriptorPool pool_;
	std::uint32_t maxSets_ = 0;
	std::uint32_t allocatedSets_ = 0;
};
