export module synodic.soul.raster.backend.vulkan:backend;

import std;
import vulkan_hpp;

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
import :fence;
import :framebuffer;

// Simple ID types to replace Entity for resource management
export using SurfaceId = std::uint64_t;
export using PassId = std::uint64_t;
export using SubPassId = std::uint64_t;

// Frame synchronization data for each frame in flight
export template<SchedulerBackend SchedulerType>
struct FrameData {
	std::optional<VulkanSemaphore> imageAvailableSemaphore;
	std::optional<VulkanSemaphore> renderFinishedSemaphore;
	std::optional<VulkanFence> inFlightFence;
	std::optional<VulkanFrameBuffer<SchedulerType>> framebuffer;

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
	std::vector<FrameData<SchedulerType>> frames;
	uvec2 size;

	SurfaceData(VulkanSurface&& surf, uvec2 surfaceSize) :
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
	std::optional<VulkanCommandBuffer> commandBuffer;
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

	//RenderPass Modification
	void CreatePassInput(Entity, Entity, Format) override;
	void CreatePassOutput(Entity, Entity, Format) override;

	Entity CreateSurface(NativeSurfaceHandle, uvec2) override;
	void UpdateSurface(Entity, uvec2) override;
	void RemoveSurface(Entity) override;
	void AttachSurface(Entity, Entity) override;
	void DetachSurface(Entity, Entity) override;

	void Compile(CommandList& commandList) override;

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

	SurfaceId GenerateSurfaceId() { return nextSurfaceId_++; }
	PassId GeneratePassId() { return nextPassId_++; }
	SubPassId GenerateSubPassId() { return nextSubPassId_++; }

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
		"VK_KHR_surface"};

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

	std::vector<std::string> deviceExtensions {"VK_KHR_swapchain"};
	// Note: Many extensions promoted to core in Vulkan 1.4

	// TODO: Device groups and multiple devices
	physicalDevices_ = instance_->EnumeratePhysicalDevices();
	devices_.push_back(VulkanDevice(scheduler_, instance_->Handle(), physicalDevices_[0].Handle(),
		validationLayers, deviceExtensions, vk::ApiVersion14));

	// TODO: One pool per device per render image set
	commandPools_.reserve(devices_.size());

	for (auto& vkDevice : devices_) {
		commandPools_.push_back(VulkanCommandPool(scheduler_, vkDevice));
	}
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
			if (currentFrame_ < surfaceData.frames.size() &&
				surfaceData.frames[currentFrame_].renderFinishedSemaphore.has_value()) {
				presentSemaphores.push_back(
					surfaceData.frames[currentFrame_].renderFinishedSemaphore->Handle());
				imageIndices.push_back(swapChain.ActiveImageIndex());
				presentSwapChains.push_back(swapChain.Handle());
			}
		}

		if (!presentSwapChains.empty()) {
			auto graphicsQueues = vkDevice.GraphicsQueues();
			if (!graphicsQueues.empty()) {
				graphicsQueues[0].Present(presentSemaphores, presentSwapChains, imageIndices);
			}
		}
	}

	// Advance to next frame
	currentFrame_ = (currentFrame_ + 1) % frameCount;
}

template<SchedulerBackend SchedulerType>
Entity VulkanRasterBackend<SchedulerType>::CreatePass(const ShaderSet& shaderSet, std::function<void(Entity)> function)
{
	// Generate new pass ID
	const PassId passId = GeneratePassId();

	// Create pass data storage
	auto [passIterator, inserted] = renderPasses_.try_emplace(passId);
	auto& passData = passIterator->second;

	// Default output attachment
	const std::uint32_t attachmentIndex = static_cast<std::uint32_t>(passData.attachments.size());

	vk::AttachmentDescription2KHR& attachment = passData.attachments.emplace_back();
	attachment.sType = vk::StructureType::eAttachmentDescription2;
	attachment.pNext = nullptr;
	attachment.flags = vk::AttachmentDescriptionFlags();
	attachment.format = vk::Format::eB8G8R8A8Unorm; // Will be updated when surface is attached
	attachment.samples = vk::SampleCountFlagBits::e1;
	attachment.loadOp = vk::AttachmentLoadOp::eClear;
	attachment.storeOp = vk::AttachmentStoreOp::eStore;
	attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachment.initialLayout = vk::ImageLayout::eUndefined;
	attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	// Create attachment reference for subpass
	vk::AttachmentReference2KHR colorAttachmentRef;
	colorAttachmentRef.sType = vk::StructureType::eAttachmentReference2;
	colorAttachmentRef.pNext = nullptr;
	colorAttachmentRef.attachment = attachmentIndex;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
	colorAttachmentRef.aspectMask = vk::ImageAspectFlagBits::eColor;

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

	// Create the VulkanSubPass object
	passData.subPasses.emplace_back(subPassData.attachmentReferences);

	// Build subpass descriptions
	std::vector<vk::SubpassDescription2KHR> subPassDescriptions;
	for (const auto& subPass : passData.subPasses) {
		subPassDescriptions.push_back(subPass.Description());
	}

	// Create the VulkanRenderPass
	passData.renderPass.emplace(devices_[0], passData.attachments, subPassDescriptions, passData.dependencies);

	// Create command buffer for this pass
	passData.commandBuffer.emplace(commandPools_.back().Handle(),
		devices_[0].Logical(), vk::CommandBufferUsageFlagBits::eSimultaneousUse,
		vk::CommandBufferLevel::ePrimary);

	// Create pipelines for each subpass
	// TODO: Load actual shaders
	std::vector<VulkanShader> emptyShaders;
	std::span<VulkanShader> shaderSpan(emptyShaders);

	for (std::size_t i = 0; i < passData.subPasses.size(); ++i) {
		passData.pipelines.emplace_back(
			devices_[0].Logical(), shaderSpan, passData.renderPass->Handle(), static_cast<std::uint32_t>(i));
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

	if (!passData.renderPass.has_value() || !passData.commandBuffer.has_value()) {
		return; // Pass not fully initialized
	}

	if (!surfaceData.swapChain.has_value()) {
		return; // No swapchain
	}

	auto& swapChain = surfaceData.swapChain.value();
	auto& renderPass = passData.renderPass.value();
	auto& commandBuffer = passData.commandBuffer.value();
	auto& commandBufferHandle = commandBuffer.Handle();

	// Check frame data
	if (currentFrame_ >= surfaceData.frames.size()) {
		return;
	}

	auto& frameData = surfaceData.frames[currentFrame_];
	if (!frameData.imageAvailableSemaphore.has_value() ||
		!frameData.renderFinishedSemaphore.has_value() ||
		!frameData.framebuffer.has_value()) {
		return;
	}

	// Acquire next swapchain image
	auto acquireResult = swapChain.AcquireImage(frameData.imageAvailableSemaphore->Handle());
	if (!acquireResult.has_value()) {
		return; // Failed to acquire image
	}

	auto swapChainSize = swapChain.Size();

	// Begin command buffer
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBufferHandle.begin(beginInfo);

	// Set up render pass begin info
	vk::ClearValue clearColor(vk::ClearColorValue(std::array<float, 4> {0.0f, 0.0f, 0.2f, 1.0f}));

	vk::RenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.renderPass = renderPass.Handle();
	renderPassBeginInfo.framebuffer = frameData.framebuffer->Handle();
	renderPassBeginInfo.renderArea.offset = vk::Offset2D{0, 0};
	renderPassBeginInfo.renderArea.extent = swapChainSize;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearColor;

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
	}

	// TODO: Process command list commands
	// The CommandList class needs iteration support to process commands here

	// End render pass and command buffer
	commandBufferHandle.endRenderPass();
	commandBufferHandle.end();

	// Submit command buffer
	vk::SubmitInfo submitInfo;
	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	vk::Semaphore waitSemaphores[] = {frameData.imageAvailableSemaphore->Handle()};
	vk::Semaphore signalSemaphores[] = {frameData.renderFinishedSemaphore->Handle()};

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBufferHandle;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	auto graphicsQueues = devices_[0].GraphicsQueues();
	if (!graphicsQueues.empty()) {
		graphicsQueues[0].Handle().submit(submitInfo);
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
Entity VulkanRasterBackend<SchedulerType>::CreateSurface(NativeSurfaceHandle nativeSurface, uvec2 size)
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

	// Create frame synchronization resources
	const auto imageCount = surfaceData.swapChain->Images().size();
	surfaceData.frames.resize(frameCount);

	for (std::uint32_t i = 0; i < frameCount; ++i) {
		auto& frame = surfaceData.frames[i];
		frame.imageAvailableSemaphore.emplace(devices_[0].Logical());
		frame.renderFinishedSemaphore.emplace(devices_[0].Logical());
		frame.inFlightFence.emplace(devices_[0].Logical());
	}

	// Return surface ID as Entity
	Entity surfaceEntity;
	std::memcpy(&surfaceEntity, &surfaceId, sizeof(SurfaceId));
	return surfaceEntity;
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::UpdateSurface(Entity surfaceEntity, uvec2 size)
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

	// Wait for device to be idle before destroying surface resources
	if (!devices_.empty()) {
		devices_[0].Synchronize();
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

	// Add surface to pass's attached surfaces
	passData.attachedSurfaces.push_back(surfaceId);

	// Create framebuffers for this surface if render pass exists
	if (passData.renderPass.has_value() && surfaceData.swapChain.has_value()) {
		auto& swapChain = surfaceData.swapChain.value();
		auto imageViews = swapChain.ImageViews();
		auto swapChainSize = swapChain.Size();

		// Create framebuffers for each frame
		for (std::size_t i = 0; i < surfaceData.frames.size() && i < imageViews.size(); ++i) {
			std::vector<vk::ImageView> attachments = {imageViews[i]};
			surfaceData.frames[i].framebuffer.emplace(
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
		default:
			// TODO: Implement all format conversions
			return vk::Format::eUndefined;
	}
}
