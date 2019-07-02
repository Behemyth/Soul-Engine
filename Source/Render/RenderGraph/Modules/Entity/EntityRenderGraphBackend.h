#pragma once

#include "Render/RenderGraph/RenderGraphModule.h"
#include <utility>

class RasterModule;

class EntityRenderGraphBackend final : public RenderGraphModule {

public:
	EntityRenderGraphBackend(std::shared_ptr<RasterModule>&, std::shared_ptr<SchedulerModule>&);
	~EntityRenderGraphBackend() override = default;

	EntityRenderGraphBackend(const EntityRenderGraphBackend&) = delete;
	EntityRenderGraphBackend(EntityRenderGraphBackend&&) noexcept = default;

	EntityRenderGraphBackend& operator=(const EntityRenderGraphBackend&) = delete;
	EntityRenderGraphBackend& operator=(EntityRenderGraphBackend&&) noexcept = default;


	void Execute() override;

	void CreateRenderPass(RenderTaskParameters&,
		std::function<std::function<void(const EntityRegistry&, CommandList&)>(
			RenderGraphBuilder&)>) override;

private:
	std::shared_ptr<RasterModule> rasterModule_;
	std::shared_ptr<EntityRegistry> entityRegistry_;
	RenderGraphBuilder builder_;
	std::vector<std::pair<Entity, std::function<void(const EntityRegistry&, CommandList&)>>>
		graphTasks_;
};