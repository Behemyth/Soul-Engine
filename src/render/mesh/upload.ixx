export module synodic.soul.render.mesh:upload;

import std;
import synodic.soul.core;
import synodic.soul.raster;  // For GPUBufferHandle
import :vertex;
import :mesh;
import :meshlet;

// Upload request - describes data to upload to GPU
export struct UploadRequest {
	const void* data = nullptr;
	std::size_t size = 0;
	std::size_t alignment = 16;  // Default to 16-byte alignment

	template<typename T>
	static UploadRequest FromSpan(std::span<const T> span, std::size_t align = alignof(T)) {
		return {span.data(), span.size_bytes(), align};
	}

	template<typename T>
	static UploadRequest FromVector(const std::vector<T>& vec, std::size_t align = alignof(T)) {
		return {vec.data(), vec.size() * sizeof(T), align};
	}
};

// Staging ring buffer configuration
export struct StagingBufferConfig {
	std::size_t size = 64 * 1024 * 1024;  // 64 MB default
	std::uint32_t frameLatency = 3;        // Frames before memory can be reused
};

// Frame fence for tracking when uploads complete
export struct FrameFence {
	std::uint64_t frameIndex = 0;
	std::size_t ringOffset = 0;   // Offset into ring buffer when frame started
	std::size_t ringSize = 0;     // Amount of staging memory used this frame
};

// Staging buffer allocator - ring buffer for upload staging
// Call BeginFrame() at start of frame, then stage uploads, then EndFrame()
// Memory is recycled after frameLatency frames
export class StagingAllocator {
public:
	StagingAllocator() = default;

	explicit StagingAllocator(const StagingBufferConfig& config)
		: config_(config)
		, writeOffset_(0)
		, frameIndex_(0)
	{
		// Reserve space for frame fences
		frameFences_.reserve(config.frameLatency + 1);
	}

	// Begin a new frame - may wait if ring buffer is full
	void BeginFrame() {
		++frameIndex_;

		// Retire completed frames and reclaim memory
		RetireCompletedFrames();

		// Record start of this frame's allocations
		currentFrameStartOffset_ = writeOffset_;
		currentFrameUsage_ = 0;
	}

	// End frame - record what memory was used
	void EndFrame() {
		if (currentFrameUsage_ > 0) {
			frameFences_.push_back({frameIndex_, currentFrameStartOffset_, currentFrameUsage_});
		}
	}

	// Allocate staging memory for an upload
	// Returns offset into staging buffer, or nullopt if buffer is full
	[[nodiscard]] std::optional<std::size_t> Allocate(std::size_t size, std::size_t alignment) {
		// Align the write offset
		std::size_t alignedOffset = (writeOffset_ + alignment - 1) & ~(alignment - 1);

		// Check if we have space (considering wrap-around)
		std::size_t endOffset = alignedOffset + size;

		if (endOffset > config_.size) {
			// Wrap around to start of buffer
			alignedOffset = 0;
			endOffset = size;

			// Check if start of buffer is free
			if (!IsRangeFree(0, size)) {
				return std::nullopt;
			}

			writeOffset_ = 0;
		} else {
			// Check if range is free
			if (!IsRangeFree(alignedOffset, size)) {
				return std::nullopt;
			}
		}

		writeOffset_ = endOffset;
		currentFrameUsage_ += size;

		return alignedOffset;
	}

	// Get total staging buffer size
	[[nodiscard]] std::size_t TotalSize() const noexcept { return config_.size; }

	// Get current frame index
	[[nodiscard]] std::uint64_t CurrentFrame() const noexcept { return frameIndex_; }

	// Mark a frame as completed on GPU (called when fence signals)
	void MarkFrameComplete(std::uint64_t frame) {
		completedFrame_ = std::max(completedFrame_, frame);
	}

private:
	// Retire frames that have completed
	void RetireCompletedFrames() {
		while (!frameFences_.empty()) {
			const auto& fence = frameFences_.front();
			if (fence.frameIndex <= completedFrame_) {
				frameFences_.erase(frameFences_.begin());
			} else {
				break;
			}
		}
	}

	// Check if a range in the ring buffer is free
	[[nodiscard]] bool IsRangeFree(std::size_t offset, std::size_t size) const {
		std::size_t endOffset = offset + size;

		for (const auto& fence : frameFences_) {
			std::size_t fenceEnd = fence.ringOffset + fence.ringSize;

			// Check for overlap
			if (fence.ringOffset < endOffset && fenceEnd > offset) {
				return false;
			}
		}

		return true;
	}

	StagingBufferConfig config_;
	std::size_t writeOffset_ = 0;
	std::size_t currentFrameStartOffset_ = 0;
	std::size_t currentFrameUsage_ = 0;
	std::uint64_t frameIndex_ = 0;
	std::uint64_t completedFrame_ = 0;
	std::vector<FrameFence> frameFences_;
};

// Mesh upload result
export struct MeshUploadResult {
	GPUBufferHandle vertexBuffer = 0;
	GPUBufferHandle indexBuffer = 0;
	GPUDeviceAddress vertexBufferGPU = InvalidGPUAddress;  // GPU device address for bindless
	GPUDeviceAddress indexBufferGPU = InvalidGPUAddress;   // GPU device address for bindless
	std::uint32_t vertexCount = 0;
	std::uint32_t indexCount = 0;
	VertexLayout layout;
	AABB bounds;

	[[nodiscard]] bool IsValid() const noexcept {
		return vertexBuffer != 0 && indexBuffer != 0;
	}

	// Convert to GPUMesh
	[[nodiscard]] GPUMesh ToGPUMesh() const {
		GPUMesh mesh;
		mesh.vertexBuffer = vertexBuffer;
		mesh.indexBuffer = indexBuffer;
		mesh.vertexBufferGPU = vertexBufferGPU;
		mesh.indexBufferGPU = indexBufferGPU;
		mesh.indexCount = indexCount;
		mesh.vertexCount = vertexCount;
		mesh.layout = layout;
		mesh.bounds = bounds;
		return mesh;
	}
};

// Buffer usage flags for mesh data
export enum class MeshBufferUsage : std::uint32_t {
	Vertex = 0x00000080,        // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
	Index = 0x00000040,         // VK_BUFFER_USAGE_INDEX_BUFFER_BIT
	TransferDst = 0x00000002,   // VK_BUFFER_USAGE_TRANSFER_DST_BIT
	TransferSrc = 0x00000001,   // VK_BUFFER_USAGE_TRANSFER_SRC_BIT
};

export constexpr MeshBufferUsage operator|(MeshBufferUsage a, MeshBufferUsage b) {
	return static_cast<MeshBufferUsage>(
		static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

// MeshUploader - uploads CPU mesh data to GPU buffers
// Uses RasterModule's buffer management for device-local storage with staging
export class MeshUploader {
public:
	explicit MeshUploader(RasterModule& raster) : raster_(raster) {}
	
	~MeshUploader() {
		// Clean up any pending uploads
		for (auto& pending : pendingUploads_) {
			if (pending.vertexBuffer != 0) {
				raster_.DestroyBuffer(pending.vertexBuffer);
			}
			if (pending.indexBuffer != 0) {
				raster_.DestroyBuffer(pending.indexBuffer);
			}
		}
	}
	
	MeshUploader(const MeshUploader&) = delete;
	MeshUploader& operator=(const MeshUploader&) = delete;
	MeshUploader(MeshUploader&&) = default;
	MeshUploader& operator=(MeshUploader&&) = default;
	
	// Upload mesh data to GPU - returns result with buffer handles
	// This is synchronous - waits for upload to complete
	[[nodiscard]] MeshUploadResult Upload(const MeshData& meshData) {
		if (!meshData.IsValid()) {
			return {};  // Invalid mesh data
		}
		
		MeshUploadResult result;
		result.vertexCount = meshData.vertexCount;
		result.indexCount = static_cast<std::uint32_t>(meshData.indices.size());
		result.layout = meshData.layout;
		result.bounds = ComputeAABB(meshData);
		
		// Create vertex buffer (device-local with BDA support for bindless rendering)
		BufferDesc vertexDesc;
		vertexDesc.size = meshData.VertexBufferSize();
		vertexDesc.usage = BufferUsage::Vertex | BufferUsage::TransferDst | BufferUsage::ShaderDeviceAddress;
		vertexDesc.memory = BufferMemory::DeviceLocal;
		
		result.vertexBuffer = raster_.CreateBuffer(vertexDesc);
		if (result.vertexBuffer == 0) {
			return {};  // Failed to create vertex buffer
		}
		
		// Create index buffer (device-local with BDA support for bindless rendering)
		BufferDesc indexDesc;
		indexDesc.size = meshData.IndexBufferSize();
		indexDesc.usage = BufferUsage::Index | BufferUsage::TransferDst | BufferUsage::ShaderDeviceAddress;
		indexDesc.memory = BufferMemory::DeviceLocal;
		
		result.indexBuffer = raster_.CreateBuffer(indexDesc);
		if (result.indexBuffer == 0) {
			raster_.DestroyBuffer(result.vertexBuffer);
			return {};  // Failed to create index buffer
		}
		
		// Upload vertex data
		raster_.UploadBufferData(result.vertexBuffer, 
			meshData.vertexData.data(), meshData.VertexBufferSize());
		
		// Upload index data
		raster_.UploadBufferData(result.indexBuffer,
			meshData.indices.data(), meshData.IndexBufferSize());
		
		// Capture GPU device addresses for bindless rendering
		result.vertexBufferGPU = raster_.GetBufferGPUAddress(result.vertexBuffer);
		result.indexBufferGPU = raster_.GetBufferGPUAddress(result.indexBuffer);
		
		return result;
	}
	
	// Upload mesh and convert directly to GPUMesh
	[[nodiscard]] GPUMesh UploadMesh(const MeshData& meshData) {
		auto result = Upload(meshData);
		return result.ToGPUMesh();
	}
	
	// Destroy a previously uploaded mesh
	void DestroyMesh(const GPUMesh& mesh) {
		if (mesh.vertexBuffer != 0) {
			raster_.DestroyBuffer(mesh.vertexBuffer);
		}
		if (mesh.indexBuffer != 0) {
			raster_.DestroyBuffer(mesh.indexBuffer);
		}
	}
	
	// Destroy mesh from upload result
	void DestroyMesh(const MeshUploadResult& result) {
		if (result.vertexBuffer != 0) {
			raster_.DestroyBuffer(result.vertexBuffer);
		}
		if (result.indexBuffer != 0) {
			raster_.DestroyBuffer(result.indexBuffer);
		}
	}
	
	// Upload mesh with meshlets for mesh shader rendering
	// Generates meshlets from mesh data and uploads all buffers
	[[nodiscard]] synodic::soul::mesh::GPUMeshletMesh UploadMeshletMesh(const MeshData& meshData, 
	                                               const synodic::soul::mesh::MeshletOptions& options = {}) {
		if (!meshData.IsValid()) {
			return {};
		}
		
		synodic::soul::mesh::GPUMeshletMesh result;
		
		// Generate meshlets from mesh data
		auto meshletData = synodic::soul::mesh::GenerateMeshlets(meshData, options);
		if (!meshletData.IsValid()) {
			return {};
		}
		
		result.meshletCount = static_cast<std::uint32_t>(meshletData.MeshletCount());
		result.vertexCount = meshData.vertexCount;
		result.vertexLayout = meshData.layout;
		result.bounds = ComputeAABB(meshData);
		
		// Create and upload vertex buffer (mesh shader still reads original vertices)
		BufferDesc vertexDesc;
		vertexDesc.size = meshData.VertexBufferSize();
		vertexDesc.usage = BufferUsage::Storage | BufferUsage::TransferDst | BufferUsage::ShaderDeviceAddress;
		vertexDesc.memory = BufferMemory::DeviceLocal;
		
		result.vertexBuffer = raster_.CreateBuffer(vertexDesc);
		if (result.vertexBuffer == 0) {
			return {};
		}
		raster_.UploadBufferData(result.vertexBuffer, meshData.vertexData.data(), meshData.VertexBufferSize());
		result.vertexBufferGPU = raster_.GetBufferGPUAddress(result.vertexBuffer);
		
		// Create and upload meshlet descriptor buffer
		BufferDesc meshletDesc;
		meshletDesc.size = meshletData.MeshletBufferSize();
		meshletDesc.usage = BufferUsage::Storage | BufferUsage::TransferDst | BufferUsage::ShaderDeviceAddress;
		meshletDesc.memory = BufferMemory::DeviceLocal;
		
		result.meshletBuffer = raster_.CreateBuffer(meshletDesc);
		if (result.meshletBuffer == 0) {
			DestroyMeshletMesh(result);
			return {};
		}
		raster_.UploadBufferData(result.meshletBuffer, meshletData.meshlets.data(), meshletData.MeshletBufferSize());
		result.meshletBufferGPU = raster_.GetBufferGPUAddress(result.meshletBuffer);
		
		// Create and upload meshlet bounds buffer (for culling)
		BufferDesc boundsDesc;
		boundsDesc.size = meshletData.BoundsBufferSize();
		boundsDesc.usage = BufferUsage::Storage | BufferUsage::TransferDst | BufferUsage::ShaderDeviceAddress;
		boundsDesc.memory = BufferMemory::DeviceLocal;
		
		result.boundsBuffer = raster_.CreateBuffer(boundsDesc);
		if (result.boundsBuffer == 0) {
			DestroyMeshletMesh(result);
			return {};
		}
		raster_.UploadBufferData(result.boundsBuffer, meshletData.bounds.data(), meshletData.BoundsBufferSize());
		result.boundsBufferGPU = raster_.GetBufferGPUAddress(result.boundsBuffer);
		
		// Create and upload vertex index buffer (meshlet -> original vertex indices)
		BufferDesc vertexIndexDesc;
		vertexIndexDesc.size = meshletData.VertexIndexBufferSize();
		vertexIndexDesc.usage = BufferUsage::Storage | BufferUsage::TransferDst | BufferUsage::ShaderDeviceAddress;
		vertexIndexDesc.memory = BufferMemory::DeviceLocal;
		
		result.vertexIndexBuffer = raster_.CreateBuffer(vertexIndexDesc);
		if (result.vertexIndexBuffer == 0) {
			DestroyMeshletMesh(result);
			return {};
		}
		raster_.UploadBufferData(result.vertexIndexBuffer, meshletData.vertexIndices.data(), meshletData.VertexIndexBufferSize());
		result.vertexIndexBufferGPU = raster_.GetBufferGPUAddress(result.vertexIndexBuffer);
		
		// Create and upload primitive index buffer (local triangle indices within meshlet)
		BufferDesc primitiveIndexDesc;
		primitiveIndexDesc.size = meshletData.PrimitiveIndexBufferSize();
		primitiveIndexDesc.usage = BufferUsage::Storage | BufferUsage::TransferDst | BufferUsage::ShaderDeviceAddress;
		primitiveIndexDesc.memory = BufferMemory::DeviceLocal;
		
		result.primitiveIndexBuffer = raster_.CreateBuffer(primitiveIndexDesc);
		if (result.primitiveIndexBuffer == 0) {
			DestroyMeshletMesh(result);
			return {};
		}
		raster_.UploadBufferData(result.primitiveIndexBuffer, meshletData.primitiveIndices.data(), meshletData.PrimitiveIndexBufferSize());
		result.primitiveIndexBufferGPU = raster_.GetBufferGPUAddress(result.primitiveIndexBuffer);
		
		return result;
	}
	
	// Destroy a meshlet mesh
	void DestroyMeshletMesh(const synodic::soul::mesh::GPUMeshletMesh& mesh) {
		if (mesh.vertexBuffer != 0) raster_.DestroyBuffer(mesh.vertexBuffer);
		if (mesh.meshletBuffer != 0) raster_.DestroyBuffer(mesh.meshletBuffer);
		if (mesh.boundsBuffer != 0) raster_.DestroyBuffer(mesh.boundsBuffer);
		if (mesh.vertexIndexBuffer != 0) raster_.DestroyBuffer(mesh.vertexIndexBuffer);
		if (mesh.primitiveIndexBuffer != 0) raster_.DestroyBuffer(mesh.primitiveIndexBuffer);
	}

private:
	RasterModule& raster_;
	std::vector<MeshUploadResult> pendingUploads_;  // For async upload tracking (future use)
};
