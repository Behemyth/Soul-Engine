#pragma once

#include "Core/Interface/Module/Module.h"

#include "Render/Raster/CommandList.h"
#include "Core/Composition/Entity/EntityRegistry.h"
#include "Parallelism/Graph/Graph.h"
#include "RenderGraphParameters.h"
#include "RenderGraphBuilder.h"
#include "Render/Raster/RenderResource.h"

#include <memory>
#include <string>
#include <functional>

class RasterModule;
class SchedulerModule;

class RenderGraphModule : public Module<RenderGraphModule> {

public:

	RenderGraphModule(std::shared_ptr<RasterModule>&, std::shared_ptr<SchedulerModule>&);
	virtual ~RenderGraphModule() = default;

	RenderGraphModule(const RenderGraphModule &) = delete;
	RenderGraphModule(RenderGraphModule &&) noexcept = default;

	RenderGraphModule& operator=(const RenderGraphModule &) = delete;
	RenderGraphModule& operator=(RenderGraphModule &&) noexcept = default;


	virtual void Execute() = 0;

	virtual void CreateRenderPass(RenderTaskParameters&,
		std::function<std::function<void(const EntityRegistry&, CommandList&)>(RenderGraphBuilder&)>) = 0;

	// Factory
	static std::shared_ptr<RenderGraphModule> CreateModule(
		std::shared_ptr<RasterModule>&,
		std::shared_ptr<SchedulerModule>&,
		std::shared_ptr<EntityRegistry>&);

protected:

	Graph renderGraph_;


};
