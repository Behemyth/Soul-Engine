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

export class VulkanRasterBackend final : public RasterModule {

public:

	static constexpr std::uint32_t frameCount = 3;

	// TODO: Restore scheduler and other shared_ptr parameters once MSVC ICE is resolved
	VulkanRasterBackend();
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

	/*
	 * Simplifies a commandList into a ready-to-execute format. Creates the opportunity for commandList reuse
	 *
	 * @param [in,out]	commandList	The commandList to simplify.
	 */

	void Compile(CommandList& commandList) override;

	[[nodiscard]] const VulkanInstance& Instance() const;


private:


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

export class VulkanSurfaceResource : public Component
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
