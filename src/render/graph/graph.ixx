export module synodic.soul.render.graph;

export import :parameter;
export import :builder;

import synodic.soul.scheduler;
import synodic.soul.core;
import synodic.soul.raster;
import synodic.soul.graph;
import :parameter;
import :builder;
import std;

export template<SchedulerBackend SchedulerType>
class RenderGraphModule {
public:
	RenderGraphModule(std::shared_ptr<RasterModule>&, SchedulerType& scheduler) :
		renderGraph_(scheduler) {
	}

	virtual ~RenderGraphModule() = default;

	RenderGraphModule(const RenderGraphModule&) = delete;
	RenderGraphModule(RenderGraphModule&&) noexcept = default;

	RenderGraphModule& operator=(const RenderGraphModule&) = delete;
	RenderGraphModule& operator=(RenderGraphModule&&) noexcept = default;

	virtual void Execute() = 0;

	virtual void CreateRenderPass(RenderTaskParameters&,
		std::function<std::function<void(const EntityRegistry&, CommandList&)>(RenderGraphBuilder&)>) = 0;

	// Factory
	static std::shared_ptr<RenderGraphModule<SchedulerType>> CreateModule(
		std::shared_ptr<RasterModule>& rasterModule,
		SchedulerType& scheduler,
		std::shared_ptr<EntityRegistry>& entityRegistry) {

		// TODO: Implement EntityRenderGraphBackend
		// return std::make_unique<EntityRenderGraphBackend>(rasterModule, scheduler, entityRegistry);
		return nullptr;
	}

protected:
	Graph<SchedulerType> renderGraph_;
};
