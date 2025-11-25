export module synodic.soul.render.graph:resource;

import std;
import synodic.soul.core;
import synodic.soul.raster;
import :pass;

// Transient resource - exists only within a single frame
export struct TransientResource {
	ResourceHandle handle = InvalidResourceHandle;
	bool isImage = true;

	// One of these will be valid depending on isImage
	TransientImageDesc imageDesc;
	TransientBufferDesc bufferDesc;

	// Aliasing info
	std::size_t aliasOffset = 0;  // Offset within aliased memory block
	std::uint32_t aliasGroup = 0; // Which aliasing group this belongs to

	// Backend-specific handle (set during execution)
	Entity backendEntity;
};

// External/imported resource - exists outside the graph
export struct ExternalResource {
	ResourceHandle handle = InvalidResourceHandle;
	Entity backendEntity;  // Pre-existing backend resource

	bool isImage = true;
	Format format = Format::RGBA;

	// Current state for synchronization
	ResourceUsage currentUsage = ResourceUsage::None;
};

// Aliasing group - resources that share the same backing memory
export struct AliasingGroup {
	std::uint32_t groupId = 0;
	std::size_t totalSize = 0;  // Total memory needed (max of all members)
	std::vector<ResourceHandle> members;

	// Backend allocation (set during compilation)
	Entity backendAllocation;
};

// Resource registry - manages all resources in the graph
export class ResourceRegistry {
public:
	ResourceRegistry() = default;
	~ResourceRegistry() = default;

	ResourceRegistry(const ResourceRegistry&) = delete;
	ResourceRegistry(ResourceRegistry&&) noexcept = default;

	ResourceRegistry& operator=(const ResourceRegistry&) = delete;
	ResourceRegistry& operator=(ResourceRegistry&&) noexcept = default;

	// Create transient image resource
	[[nodiscard]] ResourceHandle CreateTransientImage(const TransientImageDesc& desc) {
		const ResourceHandle handle = nextHandle_++;

		TransientResource resource;
		resource.handle = handle;
		resource.isImage = true;
		resource.imageDesc = desc;

		transientResources_.emplace(handle, std::move(resource));
		return handle;
	}

	// Create transient buffer resource
	[[nodiscard]] ResourceHandle CreateTransientBuffer(const TransientBufferDesc& desc) {
		const ResourceHandle handle = nextHandle_++;

		TransientResource resource;
		resource.handle = handle;
		resource.isImage = false;
		resource.bufferDesc = desc;

		transientResources_.emplace(handle, std::move(resource));
		return handle;
	}

	// Import external resource (surface, persistent buffer, etc.)
	[[nodiscard]] ResourceHandle ImportExternal(Entity backendEntity, bool isImage = true, Format format = Format::RGBA) {
		const ResourceHandle handle = nextHandle_++;

		ExternalResource resource;
		resource.handle = handle;
		resource.backendEntity = backendEntity;
		resource.isImage = isImage;
		resource.format = format;

		externalResources_.emplace(handle, std::move(resource));
		return handle;
	}

	// Lookup functions
	[[nodiscard]] bool IsTransient(ResourceHandle handle) const {
		return transientResources_.contains(handle);
	}

	[[nodiscard]] bool IsExternal(ResourceHandle handle) const {
		return externalResources_.contains(handle);
	}

	[[nodiscard]] bool IsValid(ResourceHandle handle) const {
		return IsTransient(handle) || IsExternal(handle);
	}

	[[nodiscard]] TransientResource* GetTransient(ResourceHandle handle) {
		auto it = transientResources_.find(handle);
		return it != transientResources_.end() ? &it->second : nullptr;
	}

	[[nodiscard]] const TransientResource* GetTransient(ResourceHandle handle) const {
		auto it = transientResources_.find(handle);
		return it != transientResources_.end() ? &it->second : nullptr;
	}

	[[nodiscard]] ExternalResource* GetExternal(ResourceHandle handle) {
		auto it = externalResources_.find(handle);
		return it != externalResources_.end() ? &it->second : nullptr;
	}

	[[nodiscard]] const ExternalResource* GetExternal(ResourceHandle handle) const {
		auto it = externalResources_.find(handle);
		return it != externalResources_.end() ? &it->second : nullptr;
	}

	// Get memory size for aliasing calculations
	[[nodiscard]] std::size_t GetResourceSize(ResourceHandle handle) const {
		if (auto* transient = GetTransient(handle)) {
			return transient->isImage ? transient->imageDesc.MemorySize() : transient->bufferDesc.size;
		}
		return 0;  // External resources don't participate in aliasing
	}

	// Iteration
	[[nodiscard]] const std::unordered_map<ResourceHandle, TransientResource>& TransientResources() const {
		return transientResources_;
	}

	[[nodiscard]] const std::unordered_map<ResourceHandle, ExternalResource>& ExternalResources() const {
		return externalResources_;
	}

	// Aliasing groups
	void AddAliasingGroup(AliasingGroup&& group) {
		aliasingGroups_.push_back(std::move(group));
	}

	[[nodiscard]] std::vector<AliasingGroup>& AliasingGroups() {
		return aliasingGroups_;
	}

	[[nodiscard]] const std::vector<AliasingGroup>& AliasingGroups() const {
		return aliasingGroups_;
	}

	// Reset for next frame
	void Clear() {
		transientResources_.clear();
		externalResources_.clear();
		aliasingGroups_.clear();
		nextHandle_ = 1;
	}

private:
	std::unordered_map<ResourceHandle, TransientResource> transientResources_;
	std::unordered_map<ResourceHandle, ExternalResource> externalResources_;
	std::vector<AliasingGroup> aliasingGroups_;
	ResourceHandle nextHandle_ = 1;
};

