#pragma once

#include "Core/Interface/Module/Module.h"
#include "WindowParameters.h"
#include "Display/GUI/GUIModule.h"
#include <Core/Structure/Span.h>

#include <vector>
#include <memory>

class Window;
class InputModule;
class RasterModule;

class WindowModule : public Module<WindowModule> {

public:

	WindowModule();
	virtual ~WindowModule() = default;

	WindowModule(const WindowModule&) = delete;
	WindowModule(WindowModule&&) noexcept = default;

	WindowModule& operator=(const WindowModule&) = delete;
	WindowModule& operator=(WindowModule&&) noexcept = default;


	virtual void Update() = 0;
	virtual bool Active() = 0;

	virtual void CreateWindow(const WindowParameters&, std::shared_ptr<RasterModule>&) = 0;

	virtual nonstd::span<const char*> GetRasterExtensions() = 0;

	virtual Window& MasterWindow() = 0;

	//Factory
	static std::unique_ptr<WindowModule> CreateModule(std::shared_ptr<InputModule>&);


protected:

	bool active_;


};
