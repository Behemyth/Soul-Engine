export module synodic.soul.raster.backend.vulkan:descriptor_set;

import std;
import vulkan_hpp;

import :descriptor_pool;
import :descriptor_set_layout;

// Write operation for updating descriptor set bindings
export struct DescriptorWrite {
	std::uint32_t binding = 0;
	std::uint32_t arrayElement = 0;  // For array descriptors
	
	// Buffer info (for uniform/storage buffers)
	vk::Buffer buffer;
	vk::DeviceSize offset = 0;
	vk::DeviceSize range = vk::WholeSize;
	
	// Image info (for samplers/images)
	vk::ImageView imageView;
	vk::Sampler sampler;
	vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	
	// Convenience constructors
	static DescriptorWrite Buffer(std::uint32_t binding, vk::Buffer buf, 
		vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0)
	{
		DescriptorWrite write;
		write.binding = binding;
		write.buffer = buf;
		write.offset = offset;
		write.range = size;
		return write;
	}
	
	static DescriptorWrite CombinedImageSampler(std::uint32_t binding, 
		vk::ImageView view, vk::Sampler samp,
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		DescriptorWrite write;
		write.binding = binding;
		write.imageView = view;
		write.sampler = samp;
		write.imageLayout = layout;
		return write;
	}
	
	static DescriptorWrite SampledImage(std::uint32_t binding, vk::ImageView view,
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		DescriptorWrite write;
		write.binding = binding;
		write.imageView = view;
		write.imageLayout = layout;
		return write;
	}
	
	static DescriptorWrite Sampler(std::uint32_t binding, vk::Sampler samp) {
		DescriptorWrite write;
		write.binding = binding;
		write.sampler = samp;
		return write;
	}
	
	static DescriptorWrite StorageImage(std::uint32_t binding, vk::ImageView view,
		vk::ImageLayout layout = vk::ImageLayout::eGeneral)
	{
		DescriptorWrite write;
		write.binding = binding;
		write.imageView = view;
		write.imageLayout = layout;
		return write;
	}
};

// Wrapper around vk::DescriptorSet with update utilities
export class VulkanDescriptorSet {
public:
	VulkanDescriptorSet() = default;
	
	VulkanDescriptorSet(vk::Device device, vk::DescriptorSet set, 
		const VulkanDescriptorSetLayout& layout)
		: device_(device)
		, set_(set)
		, layout_(&layout)
	{}
	
	// Update bindings in this descriptor set
	void Update(std::span<const DescriptorWrite> writes) {
		if (writes.empty() || !set_ || !layout_) return;
		
		std::vector<vk::WriteDescriptorSet> vkWrites;
		std::vector<vk::DescriptorBufferInfo> bufferInfos;
		std::vector<vk::DescriptorImageInfo> imageInfos;
		
		// Pre-allocate to avoid reallocation invalidating pointers
		bufferInfos.reserve(writes.size());
		imageInfos.reserve(writes.size());
		vkWrites.reserve(writes.size());
		
		for (const auto& write : writes) {
			// Find binding info from layout
			const DescriptorBinding* bindingInfo = nullptr;
			for (const auto& b : layout_->Bindings()) {
				if (b.binding == write.binding) {
					bindingInfo = &b;
					break;
				}
			}
			if (!bindingInfo) continue;
			
			vk::WriteDescriptorSet vkWrite;
			vkWrite.dstSet = set_;
			vkWrite.dstBinding = write.binding;
			vkWrite.dstArrayElement = write.arrayElement;
			vkWrite.descriptorCount = 1;
			vkWrite.descriptorType = bindingInfo->type;
			
			// Set buffer or image info based on descriptor type
			switch (bindingInfo->type) {
				case vk::DescriptorType::eUniformBuffer:
				case vk::DescriptorType::eStorageBuffer:
				case vk::DescriptorType::eUniformBufferDynamic:
				case vk::DescriptorType::eStorageBufferDynamic: {
					auto& info = bufferInfos.emplace_back();
					info.buffer = write.buffer;
					info.offset = write.offset;
					info.range = write.range;
					vkWrite.pBufferInfo = &info;
					break;
				}
				case vk::DescriptorType::eCombinedImageSampler: {
					auto& info = imageInfos.emplace_back();
					info.imageView = write.imageView;
					info.sampler = write.sampler;
					info.imageLayout = write.imageLayout;
					vkWrite.pImageInfo = &info;
					break;
				}
				case vk::DescriptorType::eSampledImage:
				case vk::DescriptorType::eStorageImage: {
					auto& info = imageInfos.emplace_back();
					info.imageView = write.imageView;
					info.imageLayout = write.imageLayout;
					vkWrite.pImageInfo = &info;
					break;
				}
				case vk::DescriptorType::eSampler: {
					auto& info = imageInfos.emplace_back();
					info.sampler = write.sampler;
					vkWrite.pImageInfo = &info;
					break;
				}
				default:
					continue;  // Unsupported type
			}
			
			vkWrites.push_back(vkWrite);
		}
		
		if (!vkWrites.empty()) {
			device_.updateDescriptorSets(vkWrites, {});
		}
	}
	
	// Update a single buffer binding
	void UpdateBuffer(std::uint32_t binding, vk::Buffer buffer, 
		vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0)
	{
		auto write = DescriptorWrite::Buffer(binding, buffer, size, offset);
		Update(std::span(&write, 1));
	}
	
	// Update a single combined image sampler binding
	void UpdateCombinedImageSampler(std::uint32_t binding, 
		vk::ImageView view, vk::Sampler sampler,
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		auto write = DescriptorWrite::CombinedImageSampler(binding, view, sampler, layout);
		Update(std::span(&write, 1));
	}
	
	[[nodiscard]] vk::DescriptorSet Handle() const { return set_; }
	[[nodiscard]] bool IsValid() const { return set_ != nullptr; }
	[[nodiscard]] const VulkanDescriptorSetLayout* Layout() const { return layout_; }

private:
	vk::Device device_;
	vk::DescriptorSet set_;
	const VulkanDescriptorSetLayout* layout_ = nullptr;
};

// Helper to allocate and manage descriptor sets from a pool
export class DescriptorSetAllocator {
public:
	DescriptorSetAllocator() = default;
	
	DescriptorSetAllocator(vk::Device device, VulkanDescriptorPool& pool)
		: device_(device)
		, pool_(&pool)
	{}
	
	// Allocate a descriptor set with the given layout
	[[nodiscard]] VulkanDescriptorSet Allocate(const VulkanDescriptorSetLayout& layout) {
		vk::DescriptorSet set = pool_->AllocateOne(layout.Handle());
		if (!set) {
			return {};  // Allocation failed
		}
		return VulkanDescriptorSet(device_, set, layout);
	}
	
	// Allocate multiple descriptor sets with the same layout
	[[nodiscard]] std::vector<VulkanDescriptorSet> AllocateMany(
		const VulkanDescriptorSetLayout& layout, std::uint32_t count)
	{
		std::vector<vk::DescriptorSetLayout> layouts(count, layout.Handle());
		auto sets = pool_->Allocate(layouts);
		
		std::vector<VulkanDescriptorSet> result;
		result.reserve(sets.size());
		for (auto set : sets) {
			result.emplace_back(device_, set, layout);
		}
		return result;
	}

private:
	vk::Device device_;
	VulkanDescriptorPool* pool_ = nullptr;
};
