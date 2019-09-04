#include "WindowModule.h"
#include "Core/System/Platform.h"

#include "Modules/GLFW/GLFWWindowBackend.h"
#include "Render/Raster/Modules/Vulkan/VulkanRasterBackend.h"
#include "Display/GUI/GUIModule.h"


WindowModule::WindowModule(): 
	active_(true)
{	
}

//TODO: There will only ever be one WindowModule system per Soul application. This will need to be moved to the build system per platform
std::unique_ptr<WindowModule> WindowModule::CreateModule(std::shared_ptr<InputModule>& inputModule)
{

	if constexpr (Platform::IsDesktop()) {
		return std::make_unique<GLFWWindowBackend>(inputModule);
	}
	else {
		return nullptr;
	}

}