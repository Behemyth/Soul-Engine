#pragma once

#include "Render/Raster/RasterModule.h"

#include "Surface/VulkanSwapChain.h"
#include "Surface/VulkanSurface.h"
#include "Command/VulkanCommandPool.h"
#include "Command/VulkanCommandBuffer.h"
#include "Device/VulkanPhysicalDevice.h"
#include "VulkanInstance.h"
#include "Surface/VulkanFrame.h"
#include "Core/Structure/RingBuffer.h"
#include "VulkanSubPass.h"

#include <vulkan/vulkan.hpp>
#include <glm/vec2.hpp>
#include <unordered_map>
#include <memory>

class SchedulerModule;
class WindowModule;
class VulkanDevice;
class VulkanSwapChain;

class VulkanRasterBackend final : public RasterModule {

public:
	
	static constexpr uint frameCount = 3;

	VulkanRasterBackend(std::shared_ptr<SchedulerModule>&,
		std::shared_ptr<EntityRegistry>&,
		std::shared_ptr<WindowModule>&);
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

	Entity CreateSurface(std::any, glm::uvec2) override;
	void UpdateSurface(Entity, glm::uvec2) override;
	void RemoveSurface(Entity) override;
	void AttachSurface(Entity, Entity) override;
	void DetachSurface(Entity, Entity) override;

	/*
	 * Simplifies a commandList into a ready-to-execute format. Creates the opportunity for commandList reuse
	 *
	 * @param [in,out]	commandList	The commandList to simplify.
	 */

	void Compile(CommandList& commandList) override;

	[[nodiscard]] const VulkanInstance& Instance() const;

	
private:
	

	uint8 currentFrame_;
	
	vk::Format ConvertFormat(Format);

	std::shared_ptr<EntityRegistry> entityRegistry_;

	void Draw(DrawCommand&, vk::CommandBuffer&);
	void DrawIndirect(DrawIndirectCommand&, vk::CommandBuffer&);
	void UpdateBuffer(UpdateBufferCommand&, vk::CommandBuffer&);
	void UpdateTexture(UpdateTextureCommand&, vk::CommandBuffer&);
	void CopyBuffer(CopyBufferCommand&, vk::CommandBuffer&);
	void CopyTexture(CopyTextureCommand&, vk::CommandBuffer&);

	std::vector<VulkanPhysicalDevice> physicalDevices_;
	std::vector<VulkanDevice> devices_;

	std::unordered_map<Entity, VulkanSurface> surfaces_;
	std::unordered_map<Entity, std::vector<Entity>> renderPassSurfaces_;
	
	std::vector<VulkanCommandPool> commandPools_;

	std::unordered_map<Entity, Entity> renderPassSwapChainMap_;

	std::unordered_map<Entity, std::vector<vk::AttachmentDescription2KHR>> renderPassAttachments_;
	std::unordered_map<Entity, std::vector<VulkanSubPass>> renderPassSubPasses_;
	std::unordered_map<Entity, std::vector<vk::SubpassDependency2KHR>> renderPassDependencies_;
	//map of subPasses to list of attachment IDs
	std::unordered_map<Entity, std::vector<vk::AttachmentReference2KHR>> subPassAttachmentReferences_;

	std::unordered_map < Entity, std::vector<VulkanPipeline>> pipelines_;
	std::unordered_map<Entity, VulkanRenderPass> renderPasses_;
	std::unordered_map<Entity, VulkanCommandBuffer> renderPassCommandBuffers_;
	
	//TODO: put on stack and remove deferred construction
	std::unique_ptr<VulkanInstance> instance_;


};

class VulkanSurfaceResource : public Component
{

public:
	VulkanSurfaceResource() = default;
	~VulkanSurfaceResource() = default;
	
	VulkanSurfaceResource(const VulkanSurfaceResource&) = delete;
	VulkanSurfaceResource(VulkanSurfaceResource&&) noexcept = default;

	VulkanSurfaceResource& operator=(const VulkanSurfaceResource&) = delete;
	VulkanSurfaceResource& operator=(VulkanSurfaceResource&&) noexcept = default;
	
	RingBuffer<VulkanFrame, VulkanRasterBackend::frameCount> frames;
	
};