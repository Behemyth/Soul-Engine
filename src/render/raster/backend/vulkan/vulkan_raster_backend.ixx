export module synodic.soul.raster.backend.vulkan:backend;

import std;
import vulkan_hpp;
import synodic.periapsis;

import synodic.soul.core;
import synodic.soul.raster;
import synodic.soul.scheduler;
import synodic.soul.window;
import :swapchain;
import :surface;
import :command_pool;
import :command_buffer;
import :physical_device;
import :device;
import :queue;
import :instance;
import :subpass;
import :pipeline;
import :render_pass;
import :semaphore;
import :framebuffer;
import :allocator;

// Simple ID types to replace Entity for resource management
export using SurfaceId = std::uint64_t;
export using PassId = std::uint64_t;
export using SubPassId = std::uint64_t;
export using BufferId = std::uint64_t;

// GPU buffer data holding Vulkan buffer resources
export struct BufferData {
	vk::Buffer buffer = nullptr;
	VmaAllocationHandle allocation = nullptr;
	std::size_t size = 0;
	BufferUsage usage = BufferUsage::None;
	BufferMemory memory = BufferMemory::DeviceLocal;
	void* mappedPtr = nullptr;  // Non-null if persistently mapped
	
	BufferData() = default;
	~BufferData() = default;
	
	BufferData(const BufferData&) = delete;
	BufferData(BufferData&& other) noexcept :
		buffer(other.buffer),
		allocation(other.allocation),
		size(other.size),
		usage(other.usage),
		memory(other.memory),
		mappedPtr(other.mappedPtr)
	{
		other.buffer = nullptr;
		other.allocation = nullptr;
		other.mappedPtr = nullptr;
	}
	
	BufferData& operator=(const BufferData&) = delete;
	BufferData& operator=(BufferData&& other) noexcept {
		if (this != &other) {
			buffer = other.buffer;
			allocation = other.allocation;
			size = other.size;
			usage = other.usage;
			memory = other.memory;
			mappedPtr = other.mappedPtr;
			other.buffer = nullptr;
			other.allocation = nullptr;
			other.mappedPtr = nullptr;
		}
		return *this;
	}
	
	[[nodiscard]] bool IsValid() const { return buffer != nullptr; }
};

// Depth buffer resources
export struct DepthBuffer {
	vk::Image image = nullptr;
	vk::ImageView view = nullptr;
	VmaAllocationHandle allocation = nullptr;
	vk::Format format = vk::Format::eD32Sfloat;  // D32 for best precision
	peri::math::uvec2 size = {0, 0};
	
	DepthBuffer() = default;
	~DepthBuffer() = default;
	
	DepthBuffer(const DepthBuffer&) = delete;
	DepthBuffer(DepthBuffer&& other) noexcept :
		image(other.image),
		view(other.view),
		allocation(other.allocation),
		format(other.format),
		size(other.size)
	{
		other.image = nullptr;
		other.view = nullptr;
		other.allocation = nullptr;
	}
	
	DepthBuffer& operator=(const DepthBuffer&) = delete;
	DepthBuffer& operator=(DepthBuffer&& other) noexcept {
		if (this != &other) {
			image = other.image;
			view = other.view;
			allocation = other.allocation;
			format = other.format;
			size = other.size;
			other.image = nullptr;
			other.view = nullptr;
			other.allocation = nullptr;
		}
		return *this;
	}
	
	[[nodiscard]] bool IsValid() const { return image && view; }
};

// Frame synchronization data for each frame in flight
// Uses timeline semaphores for CPU-GPU synchronization and binary semaphores for swapchain operations
export template<SchedulerBackend SchedulerType>
struct FrameData {
	std::optional<VulkanSemaphore> imageAvailableSemaphore;  // Binary - for swapchain acquire
	std::optional<VulkanSemaphore> renderFinishedSemaphore;  // Binary - for swapchain present
	std::optional<VulkanTimelineSemaphore> frameTimeline;    // Timeline - for CPU-GPU frame sync
	std::uint64_t timelineValue = 0;                          // Current timeline value for this frame
	std::optional<VulkanCommandBuffer> commandBuffer;
	bool readyToPresent = false;  // True if frame was successfully rendered

	FrameData() = default;
	~FrameData() = default;

	FrameData(const FrameData&) = delete;
	FrameData(FrameData&&) noexcept = default;

	FrameData& operator=(const FrameData&) = delete;
	FrameData& operator=(FrameData&&) noexcept = default;
};

// Surface data holding all surface-related Vulkan resources
export template<SchedulerBackend SchedulerType>
struct SurfaceData {
	VulkanSurface surface;
	std::optional<VulkanSwapChain<SchedulerType>> swapChain;
	std::vector<FrameData<SchedulerType>> frames;  // Per frame-in-flight sync resources
	std::vector<VulkanFrameBuffer<SchedulerType>> swapchainFramebuffers;  // Per swapchain image framebuffers
	DepthBuffer depthBuffer;  // Shared depth buffer for all swapchain images
	peri::math::uvec2 size;
	bool needsSwapchainRecreation = false;
	std::uint32_t currentAcquiredImageIndex = 0;  // Index of acquired swapchain image for multi-pass
	bool frameAcquired = false;  // True if swapchain image was acquired this frame

	SurfaceData(VulkanSurface&& surf, peri::math::uvec2 surfaceSize) :
		surface(std::move(surf)),
		size(surfaceSize) {}

	~SurfaceData() = default;

	SurfaceData(const SurfaceData&) = delete;
	SurfaceData(SurfaceData&&) noexcept = default;

	SurfaceData& operator=(const SurfaceData&) = delete;
	SurfaceData& operator=(SurfaceData&&) noexcept = default;
};

// Render pass data holding all pass-related resources
export template<SchedulerBackend SchedulerType>
struct RenderPassData {
	std::vector<vk::AttachmentDescription2KHR> attachments;
	std::vector<VulkanSubPass> subPasses;
	std::vector<vk::SubpassDependency2KHR> dependencies;
	std::vector<VulkanPipeline> pipelines;
	std::optional<VulkanRenderPass<SchedulerType>> renderPass;
	std::vector<SurfaceId> attachedSurfaces;

	RenderPassData() = default;
	~RenderPassData() = default;

	RenderPassData(const RenderPassData&) = delete;
	RenderPassData(RenderPassData&&) noexcept = default;

	RenderPassData& operator=(const RenderPassData&) = delete;
	RenderPassData& operator=(RenderPassData&&) noexcept = default;
};

// SubPass data
export struct SubPassData {
	PassId parentPass;
	std::vector<vk::AttachmentReference2KHR> attachmentReferences;

	SubPassData() : parentPass(0) {}
	~SubPassData() = default;
};

export template<SchedulerBackend SchedulerType>
class VulkanRasterBackend final : public RasterModule {

public:

	static constexpr std::uint32_t frameCount = 3;

	VulkanRasterBackend(SchedulerType&);
	~VulkanRasterBackend() override = default;

	VulkanRasterBackend(const VulkanRasterBackend &) = delete;
	VulkanRasterBackend(VulkanRasterBackend &&) noexcept = default;

	VulkanRasterBackend& operator=(const VulkanRasterBackend &) = delete;
	VulkanRasterBackend& operator=(VulkanRasterBackend &&) noexcept = default;

	void Present() override;

	//RenderPass Management
	Entity CreatePass(const ShaderSet&, std::function<void(Entity)>) override;
	Entity CreateSubPass(Entity, const ShaderSet&, std::function<void(Entity)>) override;
	void ExecutePass(Entity, Entity, CommandList&) override;
	void ExecutePassWithFlags(Entity, Entity, CommandList&, PassExecutionFlags) override;

	//RenderPass Modification
	void CreatePassInput(Entity, Entity, Format) override;
	void CreatePassOutput(Entity, Entity, Format) override;

	Entity CreateSurface(NativeSurfaceHandle, peri::math::uvec2) override;
	void UpdateSurface(Entity, peri::math::uvec2) override;
	void RemoveSurface(Entity) override;
	void AttachSurface(Entity, Entity) override;
	void DetachSurface(Entity, Entity) override;

	// Buffer management
	GPUBufferHandle CreateBuffer(const BufferDesc& desc) override;
	void DestroyBuffer(GPUBufferHandle handle) override;
	void UploadBufferData(GPUBufferHandle handle, const void* data, 
		std::size_t size, std::size_t offset = 0) override;
	void* MapBuffer(GPUBufferHandle handle) override;
	void UnmapBuffer(GPUBufferHandle handle) override;
	void FlushBuffer(GPUBufferHandle handle, std::size_t offset = 0, 
		std::size_t size = std::numeric_limits<std::size_t>::max()) override;

	void Compile(CommandList& commandList) override;

	// Material/lighting uniform buffer updates (call before ExecutePass)
	void UpdateMaterial(const void* materialData, std::size_t size);
	void UpdateSceneLighting(const void* lightingData, std::size_t size);

	[[nodiscard]] const VulkanInstance& Instance() const;
	[[nodiscard]] std::uint64_t InstanceHandle() const;


private:

	SchedulerType& scheduler_;
	std::uint32_t currentFrame_;

	vk::Format ConvertFormat(Format);

	// ID generators
	SurfaceId nextSurfaceId_ = 1;
	PassId nextPassId_ = 1;
	SubPassId nextSubPassId_ = 1;
	BufferId nextBufferId_ = 1;

	SurfaceId GenerateSurfaceId() { return nextSurfaceId_++; }
	PassId GeneratePassId() { return nextPassId_++; }
	SubPassId GenerateSubPassId() { return nextSubPassId_++; }
	BufferId GenerateBufferId() { return nextBufferId_++; }

	// Depth buffer management
	void CreateDepthBuffer(SurfaceData<SchedulerType>& surfaceData, peri::math::uvec2 size);
	void DestroyDepthBuffer(SurfaceData<SchedulerType>& surfaceData);

	// Command helpers
	void Draw(DrawCommand&, vk::CommandBuffer&);
	void DrawIndirect(DrawIndirectCommand&, vk::CommandBuffer&);
	void UpdateBuffer(UpdateBufferCommand&, vk::CommandBuffer&);
	void UpdateTexture(UpdateTextureCommand&, vk::CommandBuffer&);
	void CopyBuffer(CopyBufferCommand&, vk::CommandBuffer&);
	void CopyTexture(CopyTextureCommand&, vk::CommandBuffer&);

	// Vulkan infrastructure
	std::vector<VulkanPhysicalDevice> physicalDevices_;
	std::vector<VulkanDevice<SchedulerType>> devices_;
	std::vector<VulkanCommandPool<SchedulerType>> commandPools_;

	// Resource storage (replacing ECS)
	std::unordered_map<SurfaceId, SurfaceData<SchedulerType>> surfaces_;
	std::unordered_map<PassId, RenderPassData<SchedulerType>> renderPasses_;
	std::unordered_map<SubPassId, SubPassData> subPasses_;
	std::unordered_map<BufferId, BufferData> buffers_;

	// Staging buffer for uploads (device-local buffers require staging)
	std::optional<BufferData> stagingBuffer_;
	static constexpr std::size_t stagingBufferSize_ = 64 * 1024 * 1024;  // 64 MB staging buffer

	// TODO: Bindless infrastructure (No Graphics API pattern)
	// - VulkanTextureHeap for bindless textures
	// - VulkanSamplerHeap for bindless samplers  
	// - VulkanGPUAllocator for GPU memory
	// - VulkanBindlessLayout for global pipeline layout
	// These will replace the legacy descriptor infrastructure

	// TODO: put on stack and remove deferred construction
	std::unique_ptr<VulkanInstance> instance_;

};

// Template implementations
template<SchedulerBackend SchedulerType>
VulkanRasterBackend<SchedulerType>::VulkanRasterBackend(SchedulerType& scheduler):
	scheduler_(scheduler),
	currentFrame_(0)
{
	// setup Vulkan app info
	vk::ApplicationInfo appInfo;
	appInfo.apiVersion = vk::ApiVersion14;  // Vulkan 1.4
	appInfo.applicationVersion =
		vk::makeApiVersion(0, 1, 0, 0);  // TODO forward the application version here
	appInfo.pApplicationName = "Soul Engine";  // TODO forward the application name here
	appInfo.engineVersion = vk::makeApiVersion(0, 1, 0, 0);  // TODO forward the engine version here
	appInfo.pEngineName = "Soul Engine";  // TODO forward the engine name here


	std::vector<std::string> validationLayers;
	std::vector<std::string> instanceExtensions {
		"VK_KHR_surface",
		"VK_KHR_get_surface_capabilities2"};

	// Platform-specific surface extension
#ifdef _WIN32
	instanceExtensions.push_back("VK_KHR_win32_surface");
#elif defined(__linux__)
	instanceExtensions.push_back("VK_KHR_xcb_surface");
	instanceExtensions.push_back("VK_KHR_xlib_surface");
	instanceExtensions.push_back("VK_KHR_wayland_surface");
#elif defined(__APPLE__)
	instanceExtensions.push_back("VK_EXT_metal_surface");
	instanceExtensions.push_back("VK_KHR_portability_enumeration");
#endif

	if constexpr (Compiler::Debug()) {
		validationLayers.push_back("VK_LAYER_KHRONOS_validation");
		instanceExtensions.push_back("VK_EXT_debug_utils");
	}

	instance_.reset(new VulkanInstance(appInfo, validationLayers, instanceExtensions));

	std::vector<std::string> deviceExtensions {
		"VK_KHR_swapchain",
		"VK_EXT_descriptor_buffer",       // Required for bindless texture heap
		"VK_EXT_extended_dynamic_state3", // Required for dynamic blend state
	};
	// Note: Many extensions promoted to core in Vulkan 1.4
	// - VK_KHR_buffer_device_address (core in 1.2)
	// - VK_KHR_synchronization2 (core in 1.3)
	// - VK_KHR_dynamic_rendering (core in 1.3)

	// TODO: Device groups and multiple devices
	physicalDevices_ = instance_->EnumeratePhysicalDevices();
	devices_.push_back(VulkanDevice(scheduler_, instance_->Handle(), physicalDevices_[0].Handle(),
		validationLayers, deviceExtensions, vk::ApiVersion14));

	// TODO: One pool per device per render image set
	commandPools_.reserve(devices_.size());

	for (auto& vkDevice : devices_) {
		commandPools_.push_back(VulkanCommandPool(scheduler_, vkDevice));
	}

	// TODO: Initialize bindless infrastructure
	// - Create VulkanGPUAllocator (requires ReBAR)
	// - Create VulkanTextureHeap
	// - Create VulkanSamplerHeap
	// - Create VulkanBindlessLayout
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::Present()
{
	if (surfaces_.empty()) {
		return;
	}

	// Present all surfaces that have valid swapchains
	for (auto& vkDevice : devices_) {
		std::vector<vk::Semaphore> presentSemaphores;
		std::vector<vk::SwapchainKHR> presentSwapChains;
		std::vector<std::uint32_t> imageIndices;

		for (auto& [surfaceId, surfaceData] : surfaces_) {
			if (!surfaceData.swapChain.has_value()) {
				continue;
			}

			auto& swapChain = surfaceData.swapChain.value();
			if (swapChain.Device() != vkDevice.Logical()) {
				continue;
			}

			// Get the render finished semaphore for this frame
			if (currentFrame_ < surfaceData.frames.size()) {
				auto& frameData = surfaceData.frames[currentFrame_];
				if (frameData.readyToPresent && frameData.renderFinishedSemaphore.has_value()) {
					presentSemaphores.push_back(frameData.renderFinishedSemaphore->Handle());
					imageIndices.push_back(swapChain.ActiveImageIndex());
					presentSwapChains.push_back(swapChain.Handle());
					frameData.readyToPresent = false;  // Reset for next frame
				}
			}
		}

		if (!presentSwapChains.empty()) {
			auto graphicsQueues = vkDevice.GraphicsQueues();
			if (!graphicsQueues.empty()) {
				auto presentResult = graphicsQueues[0].Present(presentSemaphores, presentSwapChains, imageIndices);

				// Handle swapchain out of date - will be recreated on next frame
				if (presentResult == vk::Result::eErrorOutOfDateKHR ||
					presentResult == vk::Result::eSuboptimalKHR) {
					// TODO: Mark surfaces for swapchain recreation
				}
			}
		}
	}

	// Advance to next frame
	currentFrame_ = (currentFrame_ + 1) % frameCount;
}

// NOTE: Legacy descriptor infrastructure removed in favor of bindless pattern.
// Material/lighting data is now passed via GPU pointers in root constants.
// See: DrawWithPointersCommand, VulkanBindlessLayout, VulkanGPUAllocator

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::UpdateMaterial(const void* materialData, std::size_t size)
{
	// TODO: With bindless, material data should be allocated via GPUAllocator
	// and passed as a GPU pointer in the draw command's vertexData/pixelData.
	// This legacy function is kept as a stub for now.
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::UpdateSceneLighting(const void* lightingData, std::size_t size)
{
	// TODO: With bindless, lighting data should be allocated via GPUAllocator
	// and passed as a GPU pointer in the draw command's pixelData.
	// This legacy function is kept as a stub for now.
}

template<SchedulerBackend SchedulerType>
Entity VulkanRasterBackend<SchedulerType>::CreatePass(const ShaderSet& shaderSet, std::function<void(Entity)> function)
{
	// Generate new pass ID
	const PassId passId = GeneratePassId();

	// Create pass data storage
	auto [passIterator, inserted] = renderPasses_.try_emplace(passId);
	auto& passData = passIterator->second;

	// === Color attachment (index 0) ===
	const std::uint32_t colorAttachmentIndex = static_cast<std::uint32_t>(passData.attachments.size());

	vk::AttachmentDescription2KHR& colorAttachment = passData.attachments.emplace_back();
	colorAttachment.sType = vk::StructureType::eAttachmentDescription2;
	colorAttachment.pNext = nullptr;
	colorAttachment.flags = vk::AttachmentDescriptionFlags();
	colorAttachment.format = vk::Format::eB8G8R8A8Unorm; // Will be updated when surface is attached
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	// === Depth attachment (index 1) ===
	const std::uint32_t depthAttachmentIndex = static_cast<std::uint32_t>(passData.attachments.size());

	vk::AttachmentDescription2KHR& depthAttachment = passData.attachments.emplace_back();
	depthAttachment.sType = vk::StructureType::eAttachmentDescription2;
	depthAttachment.pNext = nullptr;
	depthAttachment.flags = vk::AttachmentDescriptionFlags();
	depthAttachment.format = vk::Format::eD32Sfloat;  // D32 for best precision
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;  // Don't need to store depth
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	// Create color attachment reference for subpass
	vk::AttachmentReference2KHR colorAttachmentRef;
	colorAttachmentRef.sType = vk::StructureType::eAttachmentReference2;
	colorAttachmentRef.pNext = nullptr;
	colorAttachmentRef.attachment = colorAttachmentIndex;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
	colorAttachmentRef.aspectMask = vk::ImageAspectFlagBits::eColor;

	// Create depth attachment reference for subpass
	vk::AttachmentReference2KHR depthAttachmentRef;
	depthAttachmentRef.sType = vk::StructureType::eAttachmentReference2;
	depthAttachmentRef.pNext = nullptr;
	depthAttachmentRef.attachment = depthAttachmentIndex;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	depthAttachmentRef.aspectMask = vk::ImageAspectFlagBits::eDepth;

	// Create default subpass
	const SubPassId subPassId = GenerateSubPassId();
	auto [subPassIterator, subPassInserted] = subPasses_.try_emplace(subPassId);
	auto& subPassData = subPassIterator->second;
	subPassData.parentPass = passId;
	subPassData.attachmentReferences.push_back(colorAttachmentRef);

	// Call user function with subpass entity (using subPassId as Entity value)
	Entity subPassEntity;
	std::memcpy(&subPassEntity, &subPassId, sizeof(SubPassId));
	function(subPassEntity);

	// Create the VulkanSubPass object with depth attachment
	passData.subPasses.emplace_back(subPassData.attachmentReferences, depthAttachmentRef);

	// Build subpass descriptions
	std::vector<vk::SubpassDescription2KHR> subPassDescriptions;
	for (const auto& subPass : passData.subPasses) {
		subPassDescriptions.push_back(subPass.Description());
	}

	// Create the VulkanRenderPass
	passData.renderPass.emplace(devices_[0], passData.attachments, subPassDescriptions, passData.dependencies);

	// Load shaders and create pipeline
	// Shader naming convention: <name>.<stage>.spv
	// TODO: Use shaderSet entities to specify shaders dynamically
	std::filesystem::path shaderDir = SOUL_SHADER_DIR;
	
	// Try PBR shaders first (for mesh rendering)
	std::filesystem::path pbrVertexPath = shaderDir / "pbr.vertex.spv";
	std::filesystem::path pbrFragmentPath = shaderDir / "pbr.fragment.spv";
	
	// Fall back to triangle shaders (for basic testing)
	std::filesystem::path triangleVertexPath = shaderDir / "triangle.vertex.spv";
	std::filesystem::path triangleFragmentPath = shaderDir / "triangle.fragment.spv";

	if (std::filesystem::exists(pbrVertexPath) && std::filesystem::exists(pbrFragmentPath) &&
		materialLayout_.has_value()) {
		// Create PBR pipeline with material/lighting descriptor sets
		std::vector<VulkanShader> shaders;
		shaders.emplace_back(devices_[0].Logical(), vk::ShaderStageFlagBits::eVertex,
			pbrVertexPath, "main");
		shaders.emplace_back(devices_[0].Logical(), vk::ShaderStageFlagBits::eFragment,
			pbrFragmentPath, "main");

		// PBR config with descriptor sets for Material (binding 0) + SceneLighting (binding 1)
		VulkanPipelineConfig pipelineConfig = VulkanPipelineConfig::PBRWithDescriptors(
			materialLayout_->Handle());

		passData.pipelines.emplace_back(
			devices_[0].Logical(),
			shaders,
			passData.renderPass->Handle(),
			0,  // subpass index
			pipelineConfig);
	}
	else if (std::filesystem::exists(triangleVertexPath) && std::filesystem::exists(triangleFragmentPath)) {
		// Fallback: simple triangle pipeline
		std::vector<VulkanShader> shaders;
		shaders.emplace_back(devices_[0].Logical(), vk::ShaderStageFlagBits::eVertex,
			triangleVertexPath, "main");
		shaders.emplace_back(devices_[0].Logical(), vk::ShaderStageFlagBits::eFragment,
			triangleFragmentPath, "main");

		// Create pipeline with no vertex input (shader uses SV_VertexID)
		VulkanPipelineConfig pipelineConfig;
		pipelineConfig.depthTest = false;
		pipelineConfig.depthWrite = false;
		pipelineConfig.cullMode = vk::CullModeFlagBits::eNone;

		passData.pipelines.emplace_back(
			devices_[0].Logical(),
			shaders,
			passData.renderPass->Handle(),
			0,  // subpass index
			pipelineConfig);
	}

	// Return pass ID as Entity
	Entity passEntity;
	std::memcpy(&passEntity, &passId, sizeof(PassId));
	return passEntity;
}

template<SchedulerBackend SchedulerType>
Entity VulkanRasterBackend<SchedulerType>::CreateSubPass(Entity renderPassEntity,
	const ShaderSet& shaderSet,
	std::function<void(Entity)> function)
{
	// Extract pass ID from Entity
	PassId passId;
	std::memcpy(&passId, &renderPassEntity, sizeof(PassId));

	auto passIt = renderPasses_.find(passId);
	if (passIt == renderPasses_.end()) {
		return Entity(); // Invalid pass
	}

	// Create new subpass
	const SubPassId subPassId = GenerateSubPassId();
	auto [subPassIterator, inserted] = subPasses_.try_emplace(subPassId);
	auto& subPassData = subPassIterator->second;
	subPassData.parentPass = passId;

	// Call user function
	Entity subPassEntity;
	std::memcpy(&subPassEntity, &subPassId, sizeof(SubPassId));
	function(subPassEntity);

	// Add subpass to parent pass
	passIt->second.subPasses.emplace_back(subPassData.attachmentReferences);

	return subPassEntity;
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::ExecutePass(Entity renderPassEntity,
	Entity surfaceEntity,
	CommandList& commandList)
{
	// Legacy single-pass execution - wraps ExecutePassWithFlags
	ExecutePassWithFlags(renderPassEntity, surfaceEntity, commandList, PassExecutionFlags::SinglePass);
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::ExecutePassWithFlags(Entity renderPassEntity,
	Entity surfaceEntity,
	CommandList& commandList,
	PassExecutionFlags flags)
{
	const bool isFirstPass = HasFlag(flags, PassExecutionFlags::FirstPass);
	const bool isLastPass = HasFlag(flags, PassExecutionFlags::LastPass);

	// Extract IDs from Entities
	PassId passId;
	std::memcpy(&passId, &renderPassEntity, sizeof(PassId));

	SurfaceId surfaceId;
	std::memcpy(&surfaceId, &surfaceEntity, sizeof(SurfaceId));

	// Find pass and surface data
	auto passIt = renderPasses_.find(passId);
	auto surfaceIt = surfaces_.find(surfaceId);

	if (passIt == renderPasses_.end() || surfaceIt == surfaces_.end()) {
		return; // Invalid pass or surface
	}

	auto& passData = passIt->second;
	auto& surfaceData = surfaceIt->second;

	if (!passData.renderPass.has_value()) {
		return; // Pass not fully initialized
	}

	if (!surfaceData.swapChain.has_value()) {
		return; // No swapchain
	}

	auto& renderPass = passData.renderPass.value();

	// Check frame data
	if (currentFrame_ >= surfaceData.frames.size()) {
		return;
	}

	auto& frameData = surfaceData.frames[currentFrame_];
	if (!frameData.imageAvailableSemaphore.has_value() ||
		!frameData.renderFinishedSemaphore.has_value() ||
		!frameData.frameTimeline.has_value() ||
		!frameData.commandBuffer.has_value()) {
		return;
	}

	// Get the per-frame command buffer
	auto& commandBuffer = frameData.commandBuffer.value();
	auto& commandBufferHandle = commandBuffer.Handle();

	// =====================================================================
	// FIRST PASS ONLY: Swapchain management, timeline wait, acquire, begin command buffer
	// =====================================================================
	if (isFirstPass) {
		// Check if swapchain needs recreation
		if (surfaceData.needsSwapchainRecreation) {
			// Wait for device idle before recreating swapchain
			devices_[0].Synchronize();

			// Update surface format
			surfaceData.surface.UpdateFormat(devices_[0]);

			// Check if surface has valid extent (not minimized)
			const auto& physicalDevice = devices_[0].Physical();
			vk::SurfaceCapabilitiesKHR surfaceCaps =
				physicalDevice.getSurfaceCapabilitiesKHR(surfaceData.surface.Handle());

			if (surfaceCaps.currentExtent.width == 0 || surfaceCaps.currentExtent.height == 0) {
				// Window is minimized or has no extent, skip this frame
				surfaceData.frameAcquired = false;
				return;
			}

			// Recreate swapchain
			VulkanSwapChain<SchedulerType>* oldSwapChain =
				surfaceData.swapChain.has_value() ? &surfaceData.swapChain.value() : nullptr;

			auto newSwapChainResult = VulkanSwapChain<SchedulerType>::Create(
				devices_[0], surfaceData.surface, false, oldSwapChain);

			if (newSwapChainResult.has_value()) {
				surfaceData.swapChain.emplace(std::move(newSwapChainResult.value()));

				// Recreate framebuffers with new swapchain image views
				auto& newSwapChain = surfaceData.swapChain.value();
				auto imageViews = newSwapChain.ImageViews();
				auto swapChainSize = newSwapChain.Size();

				// Recreate depth buffer to match new swapchain size
				CreateDepthBuffer(surfaceData, {swapChainSize.width, swapChainSize.height});

				// Clear old framebuffers and create new ones for each swapchain image
				surfaceData.swapchainFramebuffers.clear();
				for (std::size_t i = 0; i < imageViews.size(); ++i) {
					std::vector<vk::ImageView> attachments = {imageViews[i]};
					// Add depth attachment if available
					if (surfaceData.depthBuffer.IsValid()) {
						attachments.push_back(surfaceData.depthBuffer.view);
					}
					surfaceData.swapchainFramebuffers.emplace_back(
						devices_[0].Logical(),
						attachments,
						renderPass,
						swapChainSize);
				}

				surfaceData.needsSwapchainRecreation = false;
			} else {
				surfaceData.frameAcquired = false;
				return; // Recreation failed, try again next frame
			}
		}

		// Get swapchain reference AFTER potential recreation
		auto& swapChain = surfaceData.swapChain.value();

		// Wait for this frame's previous GPU work to complete using timeline semaphore
		// This ensures command buffer and binary semaphore from previous use are free
		if (frameData.timelineValue > 0) {
			frameData.frameTimeline->Wait(frameData.timelineValue);
		}

		// Acquire next swapchain image
		auto acquireResult = swapChain.AcquireImage(frameData.imageAvailableSemaphore->Handle());
		if (!acquireResult.has_value()) {
			// Mark for recreation - the device sync in recreation will handle semaphore cleanup
			surfaceData.needsSwapchainRecreation = true;
			surfaceData.frameAcquired = false;
			return;
		}

		// Store acquired image index for use by all passes this frame
		surfaceData.currentAcquiredImageIndex = acquireResult.value();
		surfaceData.frameAcquired = true;

		// Increment timeline value for this frame's submission
		frameData.timelineValue++;

		// Begin command buffer
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		commandBufferHandle.begin(beginInfo);
	}

	// =====================================================================
	// ALL PASSES: Check if frame was acquired, record render pass commands
	// =====================================================================
	if (!surfaceData.frameAcquired) {
		return; // Frame not acquired, skip rendering
	}

	// Get the acquired image index for framebuffer selection
	auto acquiredImageIndex = surfaceData.currentAcquiredImageIndex;
	if (acquiredImageIndex >= surfaceData.swapchainFramebuffers.size()) {
		// Framebuffers not ready yet
		return;
	}
	auto& framebuffer = surfaceData.swapchainFramebuffers[acquiredImageIndex];
	auto& swapChain = surfaceData.swapChain.value();
	auto swapChainSize = swapChain.Size();

	// Set up render pass begin info with color and depth clear values
	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color = vk::ClearColorValue(std::array<float, 4> {0.0f, 0.0f, 0.2f, 1.0f});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);  // Clear depth to 1.0 (far plane)

	vk::RenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.renderPass = renderPass.Handle();
	renderPassBeginInfo.framebuffer = framebuffer.Handle();
	renderPassBeginInfo.renderArea.offset = vk::Offset2D{0, 0};
	renderPassBeginInfo.renderArea.extent = swapChainSize;
	renderPassBeginInfo.clearValueCount = static_cast<std::uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	// Begin render pass
	commandBufferHandle.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	// Set viewport and scissor
	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainSize.width);
	viewport.height = static_cast<float>(swapChainSize.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandBufferHandle.setViewport(0, viewport);

	vk::Rect2D scissor;
	scissor.offset = vk::Offset2D{0, 0};
	scissor.extent = swapChainSize;
	commandBufferHandle.setScissor(0, scissor);

	// Bind pipeline if available
	if (!passData.pipelines.empty()) {
		commandBufferHandle.bindPipeline(vk::PipelineBindPoint::eGraphics, passData.pipelines[0].Handle());

		// NOTE: With bindless pattern, descriptor sets are replaced by:
		// 1. VK_EXT_descriptor_buffer for texture/sampler heaps
		// 2. Push constants for GPU pointers (RootConstants)
		// TODO: Bind descriptor buffer once bindless infrastructure is initialized

		// Process command list commands using the bindless pattern
		bool hasDrawCommands = false;
		for (std::size_t i = 0; i < commandList.CommandCount(); ++i) {
			switch (commandList.GetCommandType(i)) {
				case CommandType::DrawIndexed: {
					const auto& cmd = commandList.GetDrawIndexed(i);
					commandBufferHandle.drawIndexed(cmd.indexCount, cmd.instanceCount, 
						cmd.firstIndex, cmd.vertexOffset, cmd.firstInstance);
					hasDrawCommands = true;
					break;
				}
				case CommandType::Draw: {
					const auto& cmd = commandList.GetDraw(i);
					commandBufferHandle.draw(cmd.vertexCount, cmd.instanceCount, 
						cmd.firstVertex, cmd.firstInstance);
					hasDrawCommands = true;
					break;
				}
				case CommandType::DrawWithPointers: {
					// TODO: With bindless initialized:
					// 1. Push RootConstants with vertexData/pixelData GPU pointers
					// 2. Bind index buffer if indexed
					// 3. Issue draw call
					const auto& cmd = commandList.GetDrawWithPointers(i);
					if (cmd.IsIndexed()) {
						// TODO: Bind index buffer from GPU pointer
						commandBufferHandle.drawIndexed(cmd.indexCount, cmd.instanceCount,
							cmd.firstIndex, cmd.vertexOffset, cmd.firstInstance);
					} else {
						commandBufferHandle.draw(cmd.vertexCount, cmd.instanceCount,
							cmd.firstVertex, cmd.firstInstance);
					}
					hasDrawCommands = true;
					break;
				}
				case CommandType::Dispatch: {
					// TODO: Push root data pointer, dispatch compute
					const auto& cmd = commandList.GetDispatch(i);
					commandBufferHandle.dispatch(cmd.groupCountX, cmd.groupCountY, cmd.groupCountZ);
					break;
				}
				case CommandType::SetDepthStencilState: {
					// TODO: Use vkCmdSetDepthTestEnable, vkCmdSetDepthWriteEnable, etc.
					// (VK_EXT_extended_dynamic_state)
					break;
				}
				case CommandType::SetBlendState: {
					// TODO: Use vkCmdSetColorBlendEnableEXT, vkCmdSetColorBlendEquationEXT, etc.
					// (VK_EXT_extended_dynamic_state3)
					break;
				}
				case CommandType::Barrier: {
					// TODO: Convert BarrierCommand to vk::MemoryBarrier2
					break;
				}
				default:
					// Other command types handled elsewhere or not yet implemented
					break;
			}
		}
		
		// Fallback: if no draw commands in list, draw default triangle (shader-generated)
		if (!hasDrawCommands) {
			commandBufferHandle.draw(3, 1, 0, 0);
		}
	}

	// End this render pass
	commandBufferHandle.endRenderPass();

	// =====================================================================
	// LAST PASS ONLY: End command buffer, submit with semaphores
	// =====================================================================
	if (isLastPass) {
		// End command buffer
		commandBufferHandle.end();

		// Submit command buffer using synchronization2 with timeline semaphores
		// Wait on binary semaphore from swapchain acquire
		vk::SemaphoreSubmitInfo waitSemaphoreInfo;
		waitSemaphoreInfo.semaphore = frameData.imageAvailableSemaphore->Handle();
		waitSemaphoreInfo.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		waitSemaphoreInfo.value = 0;  // Binary semaphore
		waitSemaphoreInfo.deviceIndex = 0;

		// Signal both binary (for present) and timeline (for frame sync) semaphores
		std::array<vk::SemaphoreSubmitInfo, 2> signalSemaphoreInfos;

		// Binary semaphore for presentation
		signalSemaphoreInfos[0].semaphore = frameData.renderFinishedSemaphore->Handle();
		signalSemaphoreInfos[0].stageMask = vk::PipelineStageFlagBits2::eAllGraphics;
		signalSemaphoreInfos[0].value = 0;  // Binary semaphore
		signalSemaphoreInfos[0].deviceIndex = 0;

		// Timeline semaphore for CPU-GPU frame sync
		signalSemaphoreInfos[1].semaphore = frameData.frameTimeline->Handle();
		signalSemaphoreInfos[1].stageMask = vk::PipelineStageFlagBits2::eAllGraphics;
		signalSemaphoreInfos[1].value = frameData.timelineValue;  // Timeline value
		signalSemaphoreInfos[1].deviceIndex = 0;

		vk::CommandBufferSubmitInfo commandBufferInfo;
		commandBufferInfo.commandBuffer = commandBufferHandle;
		commandBufferInfo.deviceMask = 0;

		vk::SubmitInfo2 submitInfo;
		submitInfo.waitSemaphoreInfoCount = 1;
		submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &commandBufferInfo;
		submitInfo.signalSemaphoreInfoCount = static_cast<std::uint32_t>(signalSemaphoreInfos.size());
		submitInfo.pSignalSemaphoreInfos = signalSemaphoreInfos.data();

		auto graphicsQueues = devices_[0].GraphicsQueues();
		if (!graphicsQueues.empty()) {
			graphicsQueues[0].Handle().submit2(submitInfo);  // No fence needed - using timeline semaphore
			frameData.readyToPresent = true;  // Mark frame as ready for presentation
		}

		// Reset frame acquired flag for next frame
		surfaceData.frameAcquired = false;
	}
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::CreatePassInput(Entity passEntity, Entity resource, Format format)
{
	// Extract pass ID from Entity
	PassId passId;
	std::memcpy(&passId, &passEntity, sizeof(PassId));

	auto passIt = renderPasses_.find(passId);
	if (passIt == renderPasses_.end()) {
		return;
	}

	// TODO: Implement pass input attachment creation
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::CreatePassOutput(Entity passEntity, Entity resource, Format format)
{
	// Extract pass ID from Entity
	PassId passId;
	std::memcpy(&passId, &passEntity, sizeof(PassId));

	auto passIt = renderPasses_.find(passId);
	if (passIt == renderPasses_.end()) {
		return;
	}

	auto& passData = passIt->second;
	const std::uint32_t attachmentIndex = static_cast<std::uint32_t>(passData.attachments.size());

	vk::AttachmentDescription2KHR& attachment = passData.attachments.emplace_back();
	attachment.flags = vk::AttachmentDescriptionFlags();
	attachment.format = ConvertFormat(format);
	attachment.samples = vk::SampleCountFlagBits::e1;
	attachment.loadOp = vk::AttachmentLoadOp::eClear;
	attachment.storeOp = vk::AttachmentStoreOp::eStore;
	attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachment.initialLayout = vk::ImageLayout::eUndefined;
	attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference2KHR colorAttachmentRef;
	colorAttachmentRef.attachment = attachmentIndex;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
	colorAttachmentRef.aspectMask = vk::ImageAspectFlags();
}

template<SchedulerBackend SchedulerType>
Entity VulkanRasterBackend<SchedulerType>::CreateSurface(NativeSurfaceHandle nativeSurface, peri::math::uvec2 size)
{
	auto surfaceHandle = vk::SurfaceKHR(reinterpret_cast<VkSurfaceKHR>(nativeSurface));
	const SurfaceId surfaceId = GenerateSurfaceId();

	// Create VulkanSurface
	VulkanSurface vulkanSurface(instance_->Handle(), surfaceHandle);

	// Create SurfaceData and insert into map
	auto [surfaceIt, inserted] = surfaces_.try_emplace(surfaceId,
		std::move(vulkanSurface), size);

	if (!inserted) {
		return Entity(); // Failed to insert
	}

	auto& surfaceData = surfaceIt->second;

	// Update surface format based on device capabilities
	surfaceData.surface.UpdateFormat(devices_[0]);

	// Create swapchain
	auto swapChainResult = VulkanSwapChain<SchedulerType>::Create(
		devices_[0], surfaceData.surface, false);

	if (!swapChainResult.has_value()) {
		surfaces_.erase(surfaceId);
		return Entity(); // Failed to create swapchain
	}

	surfaceData.swapChain.emplace(std::move(swapChainResult.value()));

	// Create depth buffer matching swapchain size
	auto swapChainSize = surfaceData.swapChain->Size();
	CreateDepthBuffer(surfaceData, {swapChainSize.width, swapChainSize.height});

	// Create frame synchronization resources
	const auto imageCount = surfaceData.swapChain->Images().size();
	surfaceData.frames.resize(frameCount);

	for (std::uint32_t i = 0; i < frameCount; ++i) {
		auto& frame = surfaceData.frames[i];
		frame.imageAvailableSemaphore.emplace(devices_[0].Logical());  // Binary for swapchain acquire
		frame.renderFinishedSemaphore.emplace(devices_[0].Logical());  // Binary for swapchain present
		frame.frameTimeline.emplace(devices_[0].Logical(), 0);         // Timeline for frame sync, start at 0
		frame.timelineValue = 0;
		frame.commandBuffer.emplace(commandPools_.back().Handle(),
			devices_[0].Logical(), vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
			vk::CommandBufferLevel::ePrimary);
	}

	// Return surface ID as Entity
	Entity surfaceEntity;
	std::memcpy(&surfaceEntity, &surfaceId, sizeof(SurfaceId));
	return surfaceEntity;
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::UpdateSurface(Entity surfaceEntity, peri::math::uvec2 size)
{
	// Extract surface ID from Entity
	SurfaceId surfaceId;
	std::memcpy(&surfaceId, &surfaceEntity, sizeof(SurfaceId));

	auto surfaceIt = surfaces_.find(surfaceId);
	if (surfaceIt == surfaces_.end()) {
		return;
	}

	auto& surfaceData = surfaceIt->second;
	surfaceData.size = size;

	// Update format
	surfaceData.surface.UpdateFormat(devices_[0]);

	// Recreate swapchain
	VulkanSwapChain<SchedulerType>* oldSwapChain =
		surfaceData.swapChain.has_value() ? &surfaceData.swapChain.value() : nullptr;

	auto newSwapChainResult = VulkanSwapChain<SchedulerType>::Create(
		devices_[0], surfaceData.surface, false, oldSwapChain);

	if (newSwapChainResult.has_value()) {
		surfaceData.swapChain.emplace(std::move(newSwapChainResult.value()));
	}
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::RemoveSurface(Entity surfaceEntity)
{
	// Extract surface ID from Entity
	SurfaceId surfaceId;
	std::memcpy(&surfaceId, &surfaceEntity, sizeof(SurfaceId));

	// Wait for device to be idle before destroying surface
	if (!devices_.empty()) {
		devices_[0].Synchronize();
	}

	// Destroy depth buffer before removing surface
	auto surfaceIt = surfaces_.find(surfaceId);
	if (surfaceIt != surfaces_.end()) {
		DestroyDepthBuffer(surfaceIt->second);
	}

	surfaces_.erase(surfaceId);
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::AttachSurface(Entity renderPassEntity, Entity surfaceEntity)
{
	// Extract IDs from Entities
	PassId passId;
	std::memcpy(&passId, &renderPassEntity, sizeof(PassId));

	SurfaceId surfaceId;
	std::memcpy(&surfaceId, &surfaceEntity, sizeof(SurfaceId));

	auto passIt = renderPasses_.find(passId);
	auto surfaceIt = surfaces_.find(surfaceId);

	if (passIt == renderPasses_.end() || surfaceIt == surfaces_.end()) {
		return;
	}

	auto& passData = passIt->second;
	auto& surfaceData = surfaceIt->second;

	// Check if already attached
	for (const auto& attached : passData.attachedSurfaces) {
		if (attached == surfaceId) {
			return;  // Already attached, don't recreate framebuffers
		}
	}

	// Add surface to pass's attached surfaces
	passData.attachedSurfaces.push_back(surfaceId);

	// Create framebuffers for this surface if render pass exists
	// Only create if we don't already have framebuffers
	if (passData.renderPass.has_value() && surfaceData.swapChain.has_value() 
		&& surfaceData.swapchainFramebuffers.empty()) {
		auto& swapChain = surfaceData.swapChain.value();
		auto imageViews = swapChain.ImageViews();
		auto swapChainSize = swapChain.Size();

		// Ensure depth buffer exists and matches swapchain size
		CreateDepthBuffer(surfaceData, {swapChainSize.width, swapChainSize.height});

		// Create framebuffers for each swapchain image with color + depth attachments
		for (std::size_t i = 0; i < imageViews.size(); ++i) {
			std::vector<vk::ImageView> attachments = {imageViews[i]};
			// Add depth attachment if available
			if (surfaceData.depthBuffer.IsValid()) {
				attachments.push_back(surfaceData.depthBuffer.view);
			}
			surfaceData.swapchainFramebuffers.emplace_back(
				devices_[0].Logical(),
				attachments,
				passData.renderPass.value(),
				swapChainSize);
		}
	}
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::DetachSurface(Entity renderPassEntity, Entity surfaceEntity)
{
	// Extract IDs from Entities
	PassId passId;
	std::memcpy(&passId, &renderPassEntity, sizeof(PassId));

	SurfaceId surfaceId;
	std::memcpy(&surfaceId, &surfaceEntity, sizeof(SurfaceId));

	auto passIt = renderPasses_.find(passId);
	if (passIt == renderPasses_.end()) {
		return;
	}

	auto& attachedSurfaces = passIt->second.attachedSurfaces;
	attachedSurfaces.erase(
		std::remove(attachedSurfaces.begin(), attachedSurfaces.end(), surfaceId),
		attachedSurfaces.end());
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::Compile(CommandList&)
{
}

// Convert BufferUsage flags to Vulkan buffer usage flags
constexpr vk::BufferUsageFlags ToVulkanBufferUsage(BufferUsage usage) {
	vk::BufferUsageFlags flags;
	if (HasBufferUsage(usage, BufferUsage::Vertex)) {
		flags |= vk::BufferUsageFlagBits::eVertexBuffer;
	}
	if (HasBufferUsage(usage, BufferUsage::Index)) {
		flags |= vk::BufferUsageFlagBits::eIndexBuffer;
	}
	if (HasBufferUsage(usage, BufferUsage::Uniform)) {
		flags |= vk::BufferUsageFlagBits::eUniformBuffer;
	}
	if (HasBufferUsage(usage, BufferUsage::Storage)) {
		flags |= vk::BufferUsageFlagBits::eStorageBuffer;
	}
	if (HasBufferUsage(usage, BufferUsage::TransferSrc)) {
		flags |= vk::BufferUsageFlagBits::eTransferSrc;
	}
	if (HasBufferUsage(usage, BufferUsage::TransferDst)) {
		flags |= vk::BufferUsageFlagBits::eTransferDst;
	}
	return flags;
}

template<SchedulerBackend SchedulerType>
GPUBufferHandle VulkanRasterBackend<SchedulerType>::CreateBuffer(const BufferDesc& desc)
{
	if (desc.size == 0) {
		return 0;  // Invalid size
	}

	auto& device = devices_[0];
	auto& allocator = device.Allocator();

	// Convert usage flags - always add TransferDst for device-local buffers
	vk::BufferUsageFlags vkUsage = ToVulkanBufferUsage(desc.usage);
	if (desc.memory == BufferMemory::DeviceLocal) {
		vkUsage |= vk::BufferUsageFlagBits::eTransferDst;
	}

	// Set up VMA allocation based on memory type
	VulkanAllocationCreateInfo allocInfo;
	switch (desc.memory) {
		case BufferMemory::DeviceLocal:
			allocInfo.usage = MemoryUsage::GpuOnly;
			allocInfo.mapped = false;
			break;
		case BufferMemory::HostVisible:
			allocInfo.usage = MemoryUsage::CpuToGpu;
			allocInfo.mapped = true;
			break;
		case BufferMemory::HostCached:
			allocInfo.usage = MemoryUsage::GpuToCpu;
			allocInfo.mapped = true;
			break;
	}

	auto bufferResult = allocator.CreateBuffer(desc.size, vkUsage, allocInfo);
	if (!bufferResult.has_value()) {
		return 0;  // Failed to create buffer
	}

	// Generate buffer ID and store data
	const BufferId bufferId = GenerateBufferId();
	
	BufferData bufferData;
	bufferData.buffer = bufferResult.value().buffer;
	bufferData.allocation = bufferResult.value().allocation;
	bufferData.size = desc.size;
	bufferData.usage = desc.usage;
	bufferData.memory = desc.memory;
	bufferData.mappedPtr = bufferResult.value().mappedData;

	buffers_.emplace(bufferId, std::move(bufferData));

	return static_cast<GPUBufferHandle>(bufferId);
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::DestroyBuffer(GPUBufferHandle handle)
{
	const BufferId bufferId = static_cast<BufferId>(handle);
	auto bufferIt = buffers_.find(bufferId);
	if (bufferIt == buffers_.end()) {
		return;  // Invalid handle
	}

	auto& bufferData = bufferIt->second;
	auto& device = devices_[0];
	auto& allocator = device.Allocator();

	// Destroy buffer and free memory
	if (bufferData.buffer) {
		allocator.DestroyBuffer(bufferData.buffer, bufferData.allocation);
	}

	buffers_.erase(bufferIt);
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::UploadBufferData(GPUBufferHandle handle, 
	const void* data, std::size_t size, std::size_t offset)
{
	const BufferId bufferId = static_cast<BufferId>(handle);
	auto bufferIt = buffers_.find(bufferId);
	if (bufferIt == buffers_.end()) {
		return;  // Invalid handle
	}

	auto& bufferData = bufferIt->second;
	if (offset + size > bufferData.size) {
		return;  // Out of bounds
	}

	auto& device = devices_[0];
	auto& allocator = device.Allocator();

	// For host-visible buffers, write directly
	if (bufferData.memory != BufferMemory::DeviceLocal) {
		if (bufferData.mappedPtr) {
			std::memcpy(static_cast<char*>(bufferData.mappedPtr) + offset, data, size);
			// Flush if not cached
			if (bufferData.memory == BufferMemory::HostVisible) {
				allocator.FlushAllocation(bufferData.allocation, offset, size);
			}
		}
		return;
	}

	// For device-local buffers, use staging buffer + transfer queue
	// Create staging buffer if needed
	if (!stagingBuffer_.has_value() || stagingBuffer_->size < size) {
		if (stagingBuffer_.has_value()) {
			allocator.DestroyBuffer(stagingBuffer_->buffer, stagingBuffer_->allocation);
		}

		const std::size_t stagingSize = std::max(size, stagingBufferSize_);

		VulkanAllocationCreateInfo stagingAllocInfo;
		stagingAllocInfo.usage = MemoryUsage::CpuToGpu;
		stagingAllocInfo.mapped = true;

		auto stagingResult = allocator.CreateBuffer(stagingSize, 
			vk::BufferUsageFlagBits::eTransferSrc, stagingAllocInfo);
		if (!stagingResult.has_value()) {
			return;  // Failed to create staging buffer
		}

		BufferData stagingData;
		stagingData.buffer = stagingResult.value().buffer;
		stagingData.allocation = stagingResult.value().allocation;
		stagingData.size = stagingSize;
		stagingData.memory = BufferMemory::HostVisible;
		stagingData.mappedPtr = stagingResult.value().mappedData;
		stagingBuffer_.emplace(std::move(stagingData));
	}

	// Copy data to staging buffer
	std::memcpy(stagingBuffer_->mappedPtr, data, size);
	allocator.FlushAllocation(stagingBuffer_->allocation, 0, size);

	// Use graphics queue for transfers since our command pool is created for graphics queue family
	// TODO: Create separate transfer command pool for async transfers on dedicated transfer queue
	auto graphicsQueues = device.GraphicsQueues();
	
	if (graphicsQueues.empty()) {
		return;  // No queue available
	}
	
	VulkanQueue* queue = &graphicsQueues[0];

	// Create a one-time command buffer for the transfer
	// Use the first command pool which is created for the graphics queue family
	VulkanCommandBuffer transferCmd(commandPools_[0].Handle(),
		device.Logical(), vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
		vk::CommandBufferLevel::ePrimary);

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	transferCmd.Handle().begin(beginInfo);

	vk::BufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = offset;
	copyRegion.size = size;
	transferCmd.Handle().copyBuffer(stagingBuffer_->buffer, bufferData.buffer, copyRegion);

	transferCmd.Handle().end();

	// Submit and wait (synchronous for simplicity - could be async with timeline semaphores)
	vk::CommandBufferSubmitInfo cmdInfo;
	cmdInfo.commandBuffer = transferCmd.Handle();
	cmdInfo.deviceMask = 0;

	vk::SubmitInfo2 submitInfo;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmdInfo;

	queue->Handle().submit2(submitInfo);
	device.Logical().waitIdle();  // Simple sync - could use fence/timeline for async
}

template<SchedulerBackend SchedulerType>
void* VulkanRasterBackend<SchedulerType>::MapBuffer(GPUBufferHandle handle)
{
	const BufferId bufferId = static_cast<BufferId>(handle);
	auto bufferIt = buffers_.find(bufferId);
	if (bufferIt == buffers_.end()) {
		return nullptr;  // Invalid handle
	}

	auto& bufferData = bufferIt->second;
	
	// Device-local buffers cannot be mapped
	if (bufferData.memory == BufferMemory::DeviceLocal) {
		return nullptr;
	}

	return bufferData.mappedPtr;
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::UnmapBuffer(GPUBufferHandle handle)
{
	// Our buffers are persistently mapped, so nothing to do
	// This exists for API completeness and future flexibility
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::FlushBuffer(GPUBufferHandle handle, 
	std::size_t offset, std::size_t size)
{
	const BufferId bufferId = static_cast<BufferId>(handle);
	auto bufferIt = buffers_.find(bufferId);
	if (bufferIt == buffers_.end()) {
		return;  // Invalid handle
	}

	auto& bufferData = bufferIt->second;
	
	// Device-local buffers don't need flushing
	if (bufferData.memory == BufferMemory::DeviceLocal) {
		return;
	}

	auto& device = devices_[0];
	auto& allocator = device.Allocator();

	// Clamp size to buffer size
	if (size == std::numeric_limits<std::size_t>::max()) {
		size = bufferData.size - offset;
	}

	allocator.FlushAllocation(bufferData.allocation, offset, size);
}

template<SchedulerBackend SchedulerType>
const VulkanInstance& VulkanRasterBackend<SchedulerType>::Instance() const
{

	return *instance_;
}

template<SchedulerBackend SchedulerType>
std::uint64_t VulkanRasterBackend<SchedulerType>::InstanceHandle() const
{
	return reinterpret_cast<std::uint64_t>(static_cast<VkInstance>(instance_->Handle()));
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::Draw(DrawCommand& command, vk::CommandBuffer& commandBuffer)
{

	/*vk::Rect2D scissorRect;

	scissorRect.offset.x = command.scissorOffset.x;
	scissorRect.offset.y = command.scissorOffset.y;
	scissorRect.extent.width = command.scissorExtent.x;
	scissorRect.extent.height = command.scissorExtent.y;

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_->GetPipeline());

	vk::Buffer vertexBuffers[] = {command.vertexBuffer};
	vk::DeviceSize offsets[] = {0};

	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(command.indexBuffer, 0, vk::IndexType::eUint16);


	commandBuffer.setScissor(0, 1, &scissorRect);
	commandBuffer.drawIndexed(command.elementSize, 1, command.indexOffset, command.vertexOffset,
	0);*/
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::DrawIndirect(DrawIndirectCommand&, vk::CommandBuffer& commandBuffer)
{
	// TODO: Implement indirect drawing
	return;
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::UpdateBuffer(UpdateBufferCommand&, vk::CommandBuffer& commandBuffer)
{
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::UpdateTexture(UpdateTextureCommand&, vk::CommandBuffer& commandBuffer)
{
	// TODO: Implement texture update
	return;
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::CopyBuffer(CopyBufferCommand&, vk::CommandBuffer& commandBuffer)
{
	// TODO: Implement buffer copy
	return;
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::CopyTexture(CopyTextureCommand&, vk::CommandBuffer& commandBuffer)
{
	// TODO: Implement texture copy
	return;
}


template<SchedulerBackend SchedulerType>
vk::Format VulkanRasterBackend<SchedulerType>::ConvertFormat(Format format)
{
	switch (format) {
		case Format::RGBA:
			return vk::Format::eR8G8B8A8Srgb;
		case Format::D32_FLOAT:
			return vk::Format::eD32Sfloat;
		case Format::D24_UNORM_S8_UINT:
			return vk::Format::eD24UnormS8Uint;
		case Format::D16_UNORM:
			return vk::Format::eD16Unorm;
		default:
			// TODO: Implement all format conversions
			return vk::Format::eUndefined;
	}
}

// Create depth buffer for a surface
template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::CreateDepthBuffer(SurfaceData<SchedulerType>& surfaceData, peri::math::uvec2 size)
{
	// Don't recreate if size matches
	if (surfaceData.depthBuffer.IsValid() && 
	    surfaceData.depthBuffer.size.x == size.x && 
	    surfaceData.depthBuffer.size.y == size.y) {
		return;
	}
	
	// Destroy existing depth buffer if any
	DestroyDepthBuffer(surfaceData);
	
	auto& device = devices_[0];
	auto& allocator = device.Allocator();
	
	// Create depth image
	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = vk::Format::eD32Sfloat;  // D32 for best precision
	imageInfo.extent.width = size.x;
	imageInfo.extent.height = size.y;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	
	VulkanAllocationCreateInfo allocInfo;
	allocInfo.usage = MemoryUsage::GpuOnly;
	
	auto imageResult = allocator.CreateImage(imageInfo, allocInfo);
	if (!imageResult.has_value()) {
		return;  // Failed to create image
	}
	
	surfaceData.depthBuffer.image = imageResult.value().image;
	surfaceData.depthBuffer.allocation = imageResult.value().allocation;
	surfaceData.depthBuffer.format = vk::Format::eD32Sfloat;
	surfaceData.depthBuffer.size = size;
	
	// Create depth image view
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.image = surfaceData.depthBuffer.image;
	viewInfo.viewType = vk::ImageViewType::e2D;
	viewInfo.format = vk::Format::eD32Sfloat;
	viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	
	surfaceData.depthBuffer.view = device.Logical().createImageView(viewInfo);
}

// Destroy depth buffer resources
template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::DestroyDepthBuffer(SurfaceData<SchedulerType>& surfaceData)
{
	if (!surfaceData.depthBuffer.IsValid()) {
		return;
	}
	
	auto& device = devices_[0];
	auto& allocator = device.Allocator();
	
	// Destroy image view
	if (surfaceData.depthBuffer.view) {
		device.Logical().destroyImageView(surfaceData.depthBuffer.view);
		surfaceData.depthBuffer.view = nullptr;
	}
	
	// Destroy image and free memory
	if (surfaceData.depthBuffer.image) {
		allocator.DestroyImage(surfaceData.depthBuffer.image, surfaceData.depthBuffer.allocation);
		surfaceData.depthBuffer.image = nullptr;
		surfaceData.depthBuffer.allocation = nullptr;
	}
	
	surfaceData.depthBuffer.size = {0, 0};
}
