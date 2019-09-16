#include "RenderGraphModule.h"

#include "Display/Window/WindowModule.h"
#include "Render/RenderGraph/Modules/Entity/EntityRenderGraphBackend.h"
#include "Render/RenderGraph/RenderGraphModule.h"


RenderGraphModule::RenderGraphModule(
	std::shared_ptr<RasterModule>&,
	std::shared_ptr<SchedulerModule>& scheduler):
		renderGraph_(scheduler)
{
}

//TODO: This needs to instead be runtime loadable from shared libraries or statically linked
std::shared_ptr<RenderGraphModule> RenderGraphModule::CreateModule(
	std::shared_ptr<RasterModule>& rasterModule,
	std::shared_ptr<SchedulerModule>& scheduler,
	std::shared_ptr<EntityRegistry>& entityRegistry)
{

	return std::make_unique<EntityRenderGraphBackend>(rasterModule, scheduler, entityRegistry);

}
