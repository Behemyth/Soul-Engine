export module synodic.soul.gui;

export import :widget;
export import :layout;

import std;
import synodic.soul.core;

// Note: Forward declarations used here to avoid circular dependencies
// These types are only used in shared_ptr parameters in factory methods
export class InputModule;
export class WindowModule;
export class RenderGraphModule;

export class GUIModule : public Module<GUIModule> {

public:

	GUIModule() = default;
	virtual ~GUIModule() = default;

	GUIModule(const GUIModule&) = delete;
	GUIModule(GUIModule&&) noexcept = default;

	GUIModule& operator=(const GUIModule&) = delete;
	GUIModule& operator=(GUIModule&&) noexcept = default;


	virtual void Update(std::chrono::nanoseconds) = 0;

	// Factory
	static std::shared_ptr<GUIModule> CreateModule(std::shared_ptr<InputModule>&,
		std::shared_ptr<WindowModule>&,
		std::shared_ptr<RenderGraphModule>&);


};

// TODO: There will only ever be one DisplayModule system per Soul application. This will need to be
// moved to the build system per platform
inline std::shared_ptr<GUIModule> GUIModule::CreateModule(std::shared_ptr<InputModule>& inputModule,
	std::shared_ptr<WindowModule>& windowModule,
	std::shared_ptr<RenderGraphModule>& renderGraphModule)
{
	// ImGui support has been removed
	return nullptr;
}
