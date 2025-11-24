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
import :frame;
import :subpass;
import :pipeline;
import :render_pass;

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

	Entity CreateSurface(std::any, uvec2) override;
	void UpdateSurface(Entity, uvec2) override;
	void RemoveSurface(Entity) override;
	void AttachSurface(Entity, Entity) override;
	void DetachSurface(Entity, Entity) override;

	void Compile(CommandList& commandList) override;

	[[nodiscard]] const VulkanInstance& Instance() const;


private:

SchedulerType& scheduler_;
std::uint8_t currentFrame_;

vk::Format ConvertFormat(Format);

	std::shared_ptr<EntityRegistry> entityRegistry_;

	void Draw(DrawCommand&, vk::CommandBuffer&);
	void DrawIndirect(DrawIndirectCommand&, vk::CommandBuffer&);
	void UpdateBuffer(UpdateBufferCommand&, vk::CommandBuffer&);
	void UpdateTexture(UpdateTextureCommand&, vk::CommandBuffer&);
	void CopyBuffer(CopyBufferCommand&, vk::CommandBuffer&);
	void CopyTexture(CopyTextureCommand&, vk::CommandBuffer&);

	std::vector<VulkanPhysicalDevice> physicalDevices_;
	std::vector<VulkanDevice<SchedulerType>> devices_;

	std::unordered_map<Entity, VulkanSurface> surfaces_;
	std::unordered_map<Entity, std::vector<Entity>> renderPassSurfaces_;

	std::vector<VulkanCommandPool<SchedulerType>> commandPools_;

	std::unordered_map<Entity, Entity> renderPassSwapChainMap_;

	std::unordered_map<Entity, std::vector<vk::AttachmentDescription2KHR>> renderPassAttachments_;
	std::unordered_map<Entity, std::vector<VulkanSubPass>> renderPassSubPasses_;
	std::unordered_map<Entity, std::vector<vk::SubpassDependency2KHR>> renderPassDependencies_;
	//map of subPasses to list of attachment IDs
	std::unordered_map<Entity, std::vector<vk::AttachmentReference2KHR>> subPassAttachmentReferences_;

	std::unordered_map < Entity, std::vector<VulkanPipeline>> pipelines_;
	std::unordered_map<Entity, VulkanRenderPass<SchedulerType>> renderPasses_;
	std::unordered_map<Entity, VulkanCommandBuffer> renderPassCommandBuffers_;

	//TODO: put on stack and remove deferred construction
	std::unique_ptr<VulkanInstance> instance_;


};

export template<SchedulerBackend SchedulerType>
class VulkanSurfaceResource : public Component
{

public:
	VulkanSurfaceResource() = default;
	~VulkanSurfaceResource() = default;

	VulkanSurfaceResource(const VulkanSurfaceResource&) = delete;
	VulkanSurfaceResource(VulkanSurfaceResource&&) noexcept = default;

	VulkanSurfaceResource& operator=(const VulkanSurfaceResource&) = delete;
	VulkanSurfaceResource& operator=(VulkanSurfaceResource&&) noexcept = default;

	RingBuffer<VulkanFrame<SchedulerType>, VulkanRasterBackend<SchedulerType>::frameCount> frames;

};

// Template implementations
template<SchedulerBackend SchedulerType>
VulkanRasterBackend<SchedulerType>::VulkanRasterBackend(SchedulerType& scheduler):
scheduler_(scheduler),
currentFrame_(0)
{

	// TODO: Restore windowModule and entityRegistry initialization
	// TODO: Get window extensions from windowModule

	// setup Vulkan app info
	vk::ApplicationInfo appInfo;
	appInfo.apiVersion = vk::ApiVersion11;
	appInfo.applicationVersion =
		vk::makeApiVersion(0, 1, 0, 0);  // TODO forward the application version here
	appInfo.pApplicationName = "Soul Engine";  // TODO forward the application name here
	appInfo.engineVersion = vk::makeApiVersion(0, 1, 0, 0);  // TODO forward the engine version here
	appInfo.pEngineName = "Soul Engine";  // TODO forward the engine name here


	std::vector<std::string> validationLayers;
	std::vector<std::string> instanceExtensions {
		"VK_KHR_get_physical_device_properties2",
		"VK_KHR_get_surface_capabilities2"};

	// TODO: The display will forward the extensions needed for Vulkan
	// const auto windowExtensions = windowModule_->GetRasterExtensions();
	// instanceExtensions.insert(
	//	std::end(instanceExtensions), std::begin(windowExtensions), std::end(windowExtensions));

	if constexpr (Compiler::Debug()) {

		validationLayers.push_back("VK_LAYER_KHRONOS_validation");
		instanceExtensions.push_back("VK_EXT_debug_utils");
	}

	instance_.reset(new VulkanInstance(appInfo, validationLayers, instanceExtensions));

	std::vector<std::string> deviceExtensions {"VK_KHR_swapchain",
		"VK_KHR_external_memory", "VK_KHR_external_semaphore",
		"VK_KHR_create_renderpass2"};

	// TODO: Device groups and multiple devices
	physicalDevices_ = instance_->EnumeratePhysicalDevices();
	devices_.push_back(VulkanDevice(scheduler_, instance_->Handle(), physicalDevices_[0].Handle(),
		validationLayers, deviceExtensions, vk::ApiVersion11));

	// TODO: One pool per device per render image set
	commandPools_.reserve(devices_.size());

	for (auto& vkDevice : devices_) {
		commandPools_.push_back(VulkanCommandPool(scheduler_, vkDevice));
	}
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::Present()
{
	// TODO: Restore entityRegistry access
	// const auto swapChains = entityRegistry_->View<VulkanSwapChain>();
	// const auto surfaceResources = entityRegistry_->View<VulkanSurfaceResource>();
	// assert(swapChains.size() == surfaceResources.size());

	// TODO: Restore swapchain presentation logic
	// for (auto& vkDevice : devices_) {
	//	std::vector<vk::Semaphore> presentSemaphores;
	//	std::vector<vk::SwapchainKHR> presentSwapChains;
	//	std::vector<std::uint32_t> imageIndices;
	//	for (auto i = 0; i < swapChains.size(); ++i) {
	//		auto& swapChain = swapChains[i];
	//		if (swapChain.Device() == vkDevice.Logical()) {
	//			presentSemaphores.push_back(
	//				surfaceResources[i].frames[currentFrame_].RenderSemaphore().Handle());
	//			imageIndices.push_back(swapChain.ActiveImageIndex());
	//			presentSwapChains.push_back(swapChain.Handle());
	//		}
	//	}
	//	auto graphicsQueues = vkDevice.GraphicsQueues();
	//	bool result = graphicsQueues[0].Present(presentSemaphores, presentSwapChains, imageIndices);
	// }
}

template<SchedulerBackend SchedulerType>
Entity VulkanRasterBackend<SchedulerType>::CreatePass(const ShaderSet& ShaderSet, std::function<void(Entity)> function)
{
	// TODO: Restore entityRegistry access
	// Create new entity
	// const Entity renderPassID = entityRegistry_->CreateEntity();
	// TODO: real resource
	// const Entity resource = entityRegistry_->CreateEntity();
	const Entity renderPassID = Entity();  // Placeholder
	const Entity resource = Entity();  // Placeholder

	// Create empty pass data
	renderPassAttachments_.try_emplace(renderPassID);
	auto [subPassIterator, subPassInserted] = renderPassSubPasses_.try_emplace(renderPassID);
	renderPassDependencies_.try_emplace(renderPassID);

	auto& subPassArray = subPassIterator->second;

	// Default subPass and default output
	CreatePassOutput(renderPassID, resource, Format::RGBA);

	CreateSubPass(renderPassID, ShaderSet, [&](Entity subPassID) { function(subPassID); });

	std::vector<vk::AttachmentDescription2KHR>& subPassAttachments =
		renderPassAttachments_.at(renderPassID);

	std::vector<vk::SubpassDescription2KHR> subPassDescriptions;

	for (const auto& subPass : subPassArray) {

		subPassDescriptions.push_back(subPass.Description());

	}

	std::vector<vk::SubpassDependency2KHR>& subPassDependencies =
		renderPassDependencies_.at(renderPassID);

	//Create the renderPass object
	auto [passIterator, passInserted] = renderPasses_.try_emplace(
		renderPassID, devices_[0], subPassAttachments, subPassDescriptions, subPassDependencies);

	// TODO: command buffer per thread per ID
	renderPassCommandBuffers_.try_emplace(renderPassID, commandPools_.back().Handle(),
		devices_[0].Logical(), vk::CommandBufferUsageFlagBits::eSimultaneousUse,
		vk::CommandBufferLevel::ePrimary);

	//Create the pipeline for each subPass
	auto& renderPass = renderPasses_.at(renderPassID);

	auto [pipelineArrayIterator, pipelineArrayInserted] = pipelines_.try_emplace(renderPassID);
	pipelineArrayIterator->second.reserve(subPassArray.size());

	// TODO: Add actual shaders
	std::vector<VulkanShader> emptyShaders;
	std::span<VulkanShader> shaderSpan(emptyShaders);

	for (auto i = 0; i < subPassArray.size(); ++i)
	{

		pipelineArrayIterator->second.push_back(
			VulkanPipeline(devices_[0].Logical(), shaderSpan, renderPass.Handle(), i));

	}

	return renderPassID;
}

template<SchedulerBackend SchedulerType>
Entity VulkanRasterBackend<SchedulerType>::CreateSubPass(Entity renderPassID,
	const ShaderSet& ShaderSet,
	std::function<void(Entity)> function)
{
	// TODO: Restore entityRegistry access
	const Entity subPassID = Entity();  // Placeholder

	subPassAttachmentReferences_.try_emplace(subPassID);

	function(subPassID);

	auto& outputAttachmentReferences = subPassAttachmentReferences_.at(subPassID);

	// Create the subPass object

	std::vector<VulkanSubPass>& subPassArray = renderPassSubPasses_.at(renderPassID);
	subPassArray.emplace_back(outputAttachmentReferences);

	return subPassID;
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::ExecutePass(Entity renderPassID,
	Entity surfaceID,
	CommandList& commandList)
{
	// TODO: Restore entityRegistry access
	return;  // Temporary stub

	// TODO: Restore all execution logic once entityRegistry is available
	// vk::ClearValue clearColor(vk::ClearColorValue(std::array<float, 4> {0.0f, 0.0f, 0.0f, 1.0f}));
	// auto& swapChain = entityRegistry_->GetComponent<VulkanSwapChain>(surfaceID);
	// auto& renderPass = renderPasses_.at(renderPassID);
	// auto& commandBuffer = renderPassCommandBuffers_.at(renderPassID);
	// auto& commandBufferHandle = commandBuffer.Handle();
	// auto swapChainSize = swapChain.Size();
	// auto& surfaceResources = entityRegistry_->GetComponent<VulkanSurfaceResource>(surfaceID);
	// ... etc
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::CreatePassInput(Entity passID, Entity resource, Format format)
{
	// TODO: Implement pass input creation
	return;
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::CreatePassOutput(Entity passID, Entity resource, Format format)
{

	std::vector<vk::AttachmentDescription2KHR>& renderPassAttachments =
		renderPassAttachments_.at(passID);

	const std::uint32_t attachmentIndex = renderPassAttachments.size();

	vk::AttachmentDescription2KHR& attachment = renderPassAttachments.emplace_back();
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
Entity VulkanRasterBackend<SchedulerType>::CreateSurface(std::any anySurface, uvec2 size)
{
	// TODO: Restore entityRegistry access
	auto surfaceHandle = std::any_cast<vk::SurfaceKHR>(anySurface);
	const Entity surfaceID = Entity();  // Placeholder

	auto [surfaceIterator, didInsert] =
		surfaces_.try_emplace(surfaceID, instance_->Handle(), surfaceHandle);

	// Every surface needs a swapChain
	VulkanSurface& surface = surfaceIterator->second;
	const auto format = surface.UpdateFormat(devices_[0]);

	// TODO: Restore entityRegistry access
	// entityRegistry_->AttachComponent<VulkanSwapChain>(surfaceID, devices_[0], surface, false);
	// entityRegistry_->AttachComponent<VulkanSurfaceResource>(surfaceID);

	return surfaceID;
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::UpdateSurface(Entity surfaceID, uvec2 size)
{
	// TODO: Restore entityRegistry access
	auto& surface = surfaces_.at(surfaceID);
	const auto format = surface.UpdateFormat(devices_[0]);

	// auto& oldSwapChain = entityRegistry_->GetComponent<VulkanSwapChain>(surfaceID);
	// auto newSwapChain = VulkanSwapChain(devices_[0], surface, false, &oldSwapChain);
	// std::swap(oldSwapChain, newSwapChain);
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::RemoveSurface(Entity surfaceID)
{
	// TODO: Restore entityRegistry access
	// entityRegistry_->RemoveComponent<VulkanSwapChain>(surfaceID);
	surfaces_.erase(surfaceID);
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::AttachSurface(Entity renderPassID, Entity surfaceID)
{

	renderPassSurfaces_[renderPassID].push_back(surfaceID);
}

template<SchedulerBackend SchedulerType>
void VulkanRasterBackend<SchedulerType>::DetachSurface(Entity renderPassID, Entity surfaceID)
{

	auto& vector = renderPassSurfaces_[renderPassID];
	vector.erase(std::remove(vector.begin(), vector.end(), surfaceID), vector.end());
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
