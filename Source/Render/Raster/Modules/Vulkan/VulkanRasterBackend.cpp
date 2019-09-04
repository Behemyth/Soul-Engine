#include "VulkanRasterBackend.h"

#include "Device/VulkanDevice.h"
#include "Core/Utility/Exception/Exception.h"
#include "Core/System/Compiler.h"
#include "Core/Composition/Entity/EntityRegistry.h"
#include "Types.h"
#include "Display/Window/WindowModule.h"
#include "Surface/VulkanSwapChain.h"
#include "Render/Raster/CommandList.h"
#include "Transput/Resource/Resource.h"
#include "VulkanFramebuffer.h"

VulkanRasterBackend::VulkanRasterBackend(std::shared_ptr<SchedulerModule>& scheduler,
	std::shared_ptr<EntityRegistry>& entityRegistry,
	std::shared_ptr<WindowModule>& windowModule_):
	currentFrame_(0),
	entityRegistry_(entityRegistry)
{

	// setup Vulkan app info
	vk::ApplicationInfo appInfo;
	appInfo.apiVersion = VK_API_VERSION_1_1;
	appInfo.applicationVersion =
		VK_MAKE_VERSION(1, 0, 0);  // TODO forward the application version here
	appInfo.pApplicationName = "Soul Engine";  // TODO forward the application name here
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);  // TODO forward the engine version here
	appInfo.pEngineName = "Soul Engine";  // TODO forward the engine name here


	std::vector<std::string> validationLayers;
	std::vector<std::string> instanceExtensions {
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
		VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME};

	// The display will forward the extensions needed for Vulkan
	const auto windowExtensions = windowModule_->GetRasterExtensions();

	instanceExtensions.insert(
		std::end(instanceExtensions), std::begin(windowExtensions), std::end(windowExtensions));

	if constexpr (Compiler::Debug()) {

		validationLayers.push_back("VK_LAYER_KHRONOS_validation");
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	instance_.reset(new VulkanInstance(appInfo, validationLayers, instanceExtensions));

	std::vector<std::string> deviceExtensions {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
		VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME};

	// TODO: Device groups and multiple devices
	physicalDevices_ = instance_->EnumeratePhysicalDevices();
	devices_.emplace_back(scheduler, instance_->Handle(), physicalDevices_[0].Handle(),
		validationLayers, deviceExtensions);

	// TODO: One pool per device per render image set
	commandPools_.reserve(devices_.size());

	for (auto& vkDevice : devices_) {

		commandPools_.emplace_back(scheduler, vkDevice);
	}
}

void VulkanRasterBackend::Present()
{

	const auto swapChains = entityRegistry_->View<VulkanSwapChain>();
	const auto surfaceResources = entityRegistry_->View<VulkanSurfaceResource>();

	assert(swapChains.size() == surfaceResources.size());

	for (auto& vkDevice : devices_) {

		std::vector<vk::Semaphore> presentSemaphores;
		std::vector<vk::SwapchainKHR> presentSwapChains;
		std::vector<uint> imageIndices;

		for (auto i = 0; i < swapChains.size(); ++i) {

			auto& swapChain = swapChains[i];

			if (swapChain.Device() == vkDevice.Logical()) {

				presentSemaphores.push_back(
					surfaceResources[i].frames[currentFrame_].RenderSemaphore().Handle());
				imageIndices.push_back(swapChain.ActiveImageIndex());
				presentSwapChains.push_back(swapChain.Handle());
				
			}
		}

		auto graphicsQueues = vkDevice.GraphicsQueues();

		// TODO: Multiple present queues
		bool result = graphicsQueues[0].Present(presentSemaphores, presentSwapChains, imageIndices);
	}
}

Entity VulkanRasterBackend::CreatePass(const ShaderSet& ShaderSet, std::function<void(Entity)> function)
{

	// Create new entity
	const Entity renderPassID = entityRegistry_->CreateEntity();

	// TODO: real resource
	const Entity resource = entityRegistry_->CreateEntity();

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

	
	for (auto i = 0; i < subPassArray.size(); ++i)
	{

		pipelineArrayIterator->second.emplace_back(
			devices_[0].Logical(), subPassArray[i].Shaders(), renderPass.Handle(), i);
		
	}
		
	return renderPassID;
}

Entity VulkanRasterBackend::CreateSubPass(Entity renderPassID,
	const ShaderSet& ShaderSet,
	std::function<void(Entity)> function)
{

	const Entity subPassID = entityRegistry_->CreateEntity();

	subPassAttachmentReferences_.try_emplace(subPassID);

	function(subPassID);

	auto& outputAttachmentReferences = subPassAttachmentReferences_.at(subPassID);

	// Create the subPass object

	std::vector<VulkanSubPass>& subPassArray = renderPassSubPasses_.at(renderPassID);
	subPassArray.emplace_back(outputAttachmentReferences);
	
	return subPassID;
}

void VulkanRasterBackend::ExecutePass(Entity renderPassID,
	Entity surfaceID,
	CommandList& commandList)
{

	vk::ClearValue clearColor(vk::ClearColorValue(std::array<float, 4> {0.0f, 0.0f, 0.0f, 1.0f}));

	auto& swapChain = entityRegistry_->GetComponent<VulkanSwapChain>(surfaceID);

	auto& renderPass = renderPasses_.at(renderPassID);
	auto& commandBuffer = renderPassCommandBuffers_.at(renderPassID);
	auto& commandBufferHandle = commandBuffer.Handle();

	auto swapChainSize = swapChain.Size();

	auto& surfaceResources = entityRegistry_->GetComponent<VulkanSurfaceResource>(surfaceID);
	
	for (auto i = 0; i < surfaceResources.frames.Size(); ++i) {
		
		surfaceResources.frames[i].Framebuffer() = VulkanFrameBuffer(
			devices_[0].Logical(), swapChain.ImageViews(), renderPass, swapChainSize);
		
	}

	vk::RenderPassBeginInfo renderPassInfo;
	renderPassInfo.renderPass = renderPass.Handle();
	renderPassInfo.framebuffer = surfaceResources.frames[currentFrame_].Framebuffer().Handle();
	renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
	renderPassInfo.renderArea.extent = swapChain.Size();
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vk::SubpassBeginInfoKHR subPassInfo;
	subPassInfo.contents = vk::SubpassContents::eInline;

	commandBuffer.Begin();
	commandBufferHandle.beginRenderPass2KHR(
		renderPassInfo, subPassInfo, devices_[0].DispatchLoader());


	commandBufferHandle.endRenderPass();
	commandBuffer.End();


	//	const auto& logicalDevice = vkDevice_->GetLogical();
	//
	//	logicalDevice.waitForFences(
	//		frameFences_[currentFrame_], true, std::numeric_limits<uint64_t>::max());
	//	logicalDevice.resetFences(frameFences_[currentFrame_]);
	//
	//	vk::SubmitInfo submitInfo;
	//
	//	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	//	submitInfo.waitSemaphoreCount = 1;
	//	submitInfo.pWaitSemaphores = &presentSemaphores_[currentFrame_];
	//	submitInfo.pWaitDstStageMask = waitStages;
	//
	//	submitInfo.commandBufferCount = 1;
	//	submitInfo.pCommandBuffers = &commandBuffer_.Handle();
	//
	//	submitInfo.signalSemaphoreCount = 1;
	//	submitInfo.pSignalSemaphores = &renderSemaphores_[currentFrame_];
	//
	//	vkDevice_->GetGraphicsQueue().submit(submitInfo, frameFences_[currentFrame_]);
}

void VulkanRasterBackend::CreatePassInput(Entity passID, Entity resource, Format format)
{

	throw NotImplemented();
}

void VulkanRasterBackend::CreatePassOutput(Entity passID, Entity resource, Format format)
{

	std::vector<vk::AttachmentDescription2KHR>& renderPassAttachments =
		renderPassAttachments_.at(passID);

	const uint attachmentIndex = renderPassAttachments.size();

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

Entity VulkanRasterBackend::CreateSurface(std::any anySurface, glm::uvec2 size)
{

	auto surfaceHandle = std::any_cast<vk::SurfaceKHR>(anySurface);
	const Entity surfaceID = entityRegistry_->CreateEntity();

	auto [surfaceIterator, didInsert] =
		surfaces_.try_emplace(surfaceID, instance_->Handle(), surfaceHandle);

	// Every surface needs a swapChain
	VulkanSurface& surface = surfaceIterator->second;
	const auto format = surface.UpdateFormat(devices_[0]);

	entityRegistry_->AttachComponent<VulkanSwapChain>(surfaceID, devices_[0], surface, false);

	// create the frame storage for the surface
	entityRegistry_->AttachComponent<VulkanSurfaceResource>(surfaceID);


	return surfaceID;
}

void VulkanRasterBackend::UpdateSurface(Entity surfaceID, glm::uvec2 size)
{

	auto& surface = surfaces_.at(surfaceID);
	const auto format = surface.UpdateFormat(devices_[0]);

	auto& oldSwapChain = entityRegistry_->GetComponent<VulkanSwapChain>(surfaceID);
	auto newSwapChain = VulkanSwapChain(devices_[0], surface, false, &oldSwapChain);

	std::swap(oldSwapChain, newSwapChain);
}

void VulkanRasterBackend::RemoveSurface(Entity surfaceID)
{

	entityRegistry_->RemoveComponent<VulkanSwapChain>(surfaceID);
	surfaces_.erase(surfaceID);
}

void VulkanRasterBackend::AttachSurface(Entity renderPassID, Entity surfaceID)
{

	renderPassSurfaces_[renderPassID].push_back(surfaceID);
}

void VulkanRasterBackend::DetachSurface(Entity renderPassID, Entity surfaceID)
{

	auto& vector = renderPassSurfaces_[renderPassID];
	vector.erase(std::remove(vector.begin(), vector.end(), surfaceID), vector.end());
}

void VulkanRasterBackend::Compile(CommandList&)
{
}

const VulkanInstance& VulkanRasterBackend::Instance() const
{

	return *instance_;
}

void VulkanRasterBackend::Draw(DrawCommand& command, vk::CommandBuffer& commandBuffer)
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

void VulkanRasterBackend::DrawIndirect(DrawIndirectCommand&, vk::CommandBuffer& commandBuffer)
{

	throw NotImplemented();
}
void VulkanRasterBackend::UpdateBuffer(UpdateBufferCommand&, vk::CommandBuffer& commandBuffer)
{
}
void VulkanRasterBackend::UpdateTexture(UpdateTextureCommand&, vk::CommandBuffer& commandBuffer)
{

	throw NotImplemented();
}
void VulkanRasterBackend::CopyBuffer(CopyBufferCommand&, vk::CommandBuffer& commandBuffer)
{

	throw NotImplemented();
}
void VulkanRasterBackend::CopyTexture(CopyTextureCommand&, vk::CommandBuffer& commandBuffer)
{

	throw NotImplemented();
}


vk::Format VulkanRasterBackend::ConvertFormat(Format format)
{
	switch (format) {
		case Format::RGBA:
			return vk::Format::eR8G8B8A8Srgb;
		default:
			throw NotImplemented();
			return vk::Format::eUndefined;
	}
}
