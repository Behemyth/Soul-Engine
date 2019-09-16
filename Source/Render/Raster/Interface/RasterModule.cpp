#include "RasterModule.h"

#include "Display/Window/WindowModule.h"
#include "Modules/Vulkan/VulkanRasterBackend.h"

RasterModule::RasterModule()
{
}


//TODO: This needs to instead be runtime loadable from shared libraries or statically linked
std::shared_ptr<RasterModule> RasterModule::CreateModule(
	std::shared_ptr<SchedulerModule>& scheduler,
	std::shared_ptr<EntityRegistry>& entityRegistry,
	std::shared_ptr<WindowModule>& windowModule)
{

	return std::make_unique<VulkanRasterBackend>(scheduler, entityRegistry, windowModule);

}
