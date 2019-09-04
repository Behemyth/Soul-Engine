#include "GUIModule.h"

#include "Core/System/Platform.h"
#include "Modules/Imgui/ImguiBackend.h"


// TODO: There will only ever be one DisplayModule system per Soul application. This will need to be
// moved to the build system per platform
std::shared_ptr<GUIModule> GUIModule::CreateModule(std::shared_ptr<InputModule>& inputModule,
	std::shared_ptr<WindowModule>& windowModule,
	std::shared_ptr<RenderGraphModule>& renderGraphModule)
{

	if constexpr (Platform::IsDesktop()) {
		return std::make_shared<ImguiBackend>(inputModule, windowModule, renderGraphModule);
	}
	else {
		return nullptr;
	}
}