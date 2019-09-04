#include "InputModule.h"
#include "Core/System/Platform.h"

#include "Modules/GLFW/GLFWInputBackend.h"

void InputModule::AddMousePositionCallback(std::function<void(double, double)> callback)
{

	mousePositionCallbacks_.push_back(callback);

}

void InputModule::AddMouseButtonCallback(std::function<void(uint, ButtonState)> callback)
{

	mouseButtonCallbacks_.push_back(callback);

}


//TODO: There will only ever be one DisplayModule system per Soul application. This will need to be moved to the build system per platform
std::unique_ptr<InputModule> InputModule::CreateModule()
{

	if constexpr (Platform::IsDesktop()) {
		return std::make_unique<GLFWInputBackend>();
	}
	else {
		return nullptr;
	}

}