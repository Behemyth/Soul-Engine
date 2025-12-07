export module synodic.soul.render.graph.backend.standard;

import synodic.soul.render.graph;
import synodic.soul.raster;
import synodic.soul.core;
import synodic.soul.scheduler;
import synodic.library;
import std;

// Execution context for pass callbacks
export struct PassExecutionContext {
	const EntityRegistry& entities;
	CommandList& commands;
	ResourceHandle surfaceTarget = InvalidResourceHandle;
	synodic::math::uvec2 renderArea = {0, 0};
};

// Standard render graph backend implementation
// Executes the compiled render graph using the raster RHI
export class StandardRenderGraphBackend {
public:
	StandardRenderGraphBackend() = default;

	~StandardRenderGraphBackend() = default;

	StandardRenderGraphBackend(const StandardRenderGraphBackend&) = delete;
	StandardRenderGraphBackend(StandardRenderGraphBackend&&) noexcept = default;

	StandardRenderGraphBackend& operator=(const StandardRenderGraphBackend&) = delete;
	StandardRenderGraphBackend& operator=(StandardRenderGraphBackend&&) noexcept = default;

	// Initialize with raster module (call once after construction)
	void Initialize(RasterModule* raster) {
		rasterModule_ = raster;
	}

	// Check if initialized
	[[nodiscard]] bool IsInitialized() const { return rasterModule_ != nullptr; }

	// Access the frame graph for building passes
	[[nodiscard]] RenderGraph& FrameGraph() { return frameGraph_; }
	[[nodiscard]] const RenderGraph& FrameGraph() const { return frameGraph_; }

	// Clear the graph for a new frame
	void BeginFrame() {
		frameGraph_.Clear();
		passCallbacks_.clear();
	}

	// Import an external surface resource
	[[nodiscard]] ResourceHandle ImportSurface(Entity surfaceEntity) {
		return frameGraph_.ImportExternal(surfaceEntity, true, Format::RGBA);
	}

	// Add a render pass using the fluent builder
	std::uint32_t AddPass(RenderPassDesc&& desc) {
		return frameGraph_.AddRenderPass(std::move(desc));
	}

	// Add a render pass with an execution callback
	template<typename Callback>
	std::uint32_t AddPassWithCallback(RenderPassDesc&& desc, Callback&& callback) {
		const std::uint32_t passIndex = frameGraph_.AddRenderPass(std::move(desc));
		passCallbacks_.emplace(passIndex, std::forward<Callback>(callback));
		return passIndex;
	}

	// Compile and execute the frame graph for a surface
	void Execute(Entity surfaceEntity, synodic::math::uvec2 surfaceSize) {
		if (!rasterModule_) {
			return;
		}

		// Compile the graph
		const auto& compiled = frameGraph_.Compile();
		if (!compiled.isValid) {
			return;  // Graph has errors (cycles, etc.)
		}

		// Allocate transient resources with aliasing
		AllocateTransientResources(compiled);

		// Execute passes in topological order
		CommandList commandList;
		const std::size_t passCount = compiled.executionOrder.size();

		for (std::size_t i = 0; i < passCount; ++i) {
			const std::uint32_t passIndex = compiled.executionOrder[i];

			// Insert barriers before this pass
			InsertBarriers(compiled, passIndex, commandList);

			// Determine execution flags based on position in pass sequence
			PassExecutionFlags flags = PassExecutionFlags::None;
			const bool isFirst = (i == 0);
			const bool isLast = (i == passCount - 1);
			
			if (isFirst && isLast) {
				flags = PassExecutionFlags::SinglePass;
			} else if (isFirst) {
				flags = PassExecutionFlags::FirstPass;
			} else if (isLast) {
				flags = PassExecutionFlags::LastPass;
			}

			// Execute the pass
			if (passIndex < frameGraph_.RenderPasses().size()) {
				ExecuteRenderPass(passIndex, surfaceEntity, surfaceSize, commandList, flags);
			}
			// TODO: Handle compute and transfer passes
		}
	}

	// Present all surfaces
	void Present() {
		if (rasterModule_) {
			rasterModule_->Present();
		}
	}

private:
	void AllocateTransientResources(const CompiledRenderGraph& compiled) {
		// Allocate backing memory for aliasing groups
		for (const auto& group : compiled.aliasingGroups) {
			if (group.totalSize == 0) continue;

			// TODO: Use VMA to allocate a single memory block for each group
			// Resources in the same aliasing group share memory (non-overlapping lifetimes)
			for (const auto& handle : group.members) {
				if (auto* transient = frameGraph_.Resources().GetTransient(handle)) {
					// Create backend resource with aliased memory
					// TODO: Implement actual VMA allocation
				}
			}
		}
	}

	void InsertBarriers(const CompiledRenderGraph& compiled, std::uint32_t beforePass, CommandList& commands) {
		// Find barriers that need to execute before this pass
		for (const auto& barrier : compiled.barriers) {
			if (barrier.beforePass == beforePass) {
				// TODO: Record pipeline barrier in command list
				// This maps to vkCmdPipelineBarrier2 with synchronization2
			}
		}
	}

	void ExecuteRenderPass(
		std::uint32_t passIndex,
		Entity surfaceEntity,
		synodic::math::uvec2 surfaceSize,
		CommandList& commandList,
		PassExecutionFlags flags) {

		const auto& passDesc = frameGraph_.RenderPasses()[passIndex];

		// Get or create the Vulkan render pass
		Entity vulkanPass = CreateOrGetVulkanPass(passIndex, passDesc.shaders);

		// Attach surface to pass if not already attached
		auto& attachedSurfaces = passAttachedSurfaces_[passIndex];
		if (attachedSurfaces.find(surfaceEntity) == attachedSurfaces.end()) {
			rasterModule_->AttachSurface(vulkanPass, surfaceEntity);
			attachedSurfaces.insert(surfaceEntity);
		}

		// Execute callback if registered
		auto callbackIt = passCallbacks_.find(passIndex);
		if (callbackIt != passCallbacks_.end()) {
			EntityRegistry dummyRegistry;  // TODO: Get actual registry
			PassExecutionContext context{
				dummyRegistry,
				commandList,
				passDesc.colorOutputs.empty() ? InvalidResourceHandle : passDesc.colorOutputs[0].handle,
				surfaceSize
			};
			callbackIt->second(context);
		}

		// Execute via raster module with flags for multi-pass support
		rasterModule_->ExecutePassWithFlags(vulkanPass, surfaceEntity, commandList, flags);
	}

	Entity CreateOrGetVulkanPass(std::uint32_t passIndex, const ShaderSet& shaders) {
		// Check cache
		auto it = vulkanPassCache_.find(passIndex);
		if (it != vulkanPassCache_.end()) {
			return it->second;
		}

		// Create new Vulkan render pass
		Entity newPass = rasterModule_->CreatePass(shaders, [](Entity) {});
		vulkanPassCache_[passIndex] = newPass;
		return newPass;
	}

	RasterModule* rasterModule_ = nullptr;
	RenderGraph frameGraph_;

	// Pass execution callbacks
	std::unordered_map<std::uint32_t, std::function<void(PassExecutionContext&)>> passCallbacks_;

	// Vulkan pass cache (avoids recreating passes every frame)
	std::unordered_map<std::uint32_t, Entity> vulkanPassCache_;

	// Track which surfaces are attached to which passes
	std::unordered_map<std::uint32_t, std::unordered_set<Entity>> passAttachedSurfaces_;
};
