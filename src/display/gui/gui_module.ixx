export module synodic.soul.gui;

// Core UI types and utilities
export import :ui_types;
export import :ui_state;
export import :draw_list;

// Widget hierarchy
export import :widget;
export import :layout;

// Font and rendering
export import :font_atlas;
export import :ui_renderer;

// Concrete widgets
export import :dropdown;

import std;
import synodic.soul.core;

// Note: Forward declarations used here to avoid circular dependencies
// These types are only used in shared_ptr parameters in factory methods
export class InputModule;
export class WindowModule;
export class RenderGraphModule;

// ============================================================================
// GUI Module Interface
// ============================================================================

export class GUIModule : public Module<GUIModule> {

public:
	GUIModule() = default;
	virtual ~GUIModule() = default;

	GUIModule(const GUIModule&) = delete;
	GUIModule(GUIModule&&) noexcept = default;

	GUIModule& operator=(const GUIModule&) = delete;
	GUIModule& operator=(GUIModule&&) noexcept = default;

	/// Update the GUI system (layout, animation, etc.)
	virtual void Update(std::chrono::nanoseconds deltaTime) = 0;

	/// Get the draw list for render graph integration
	/// Call after Update() to get the current frame's draw commands
	[[nodiscard]] virtual const UIDrawList& GetDrawList() const = 0;

	/// Get render data package for the UI render pass
	[[nodiscard]] virtual UIRenderData GetRenderData() const = 0;

	/// Check if UI needs redrawing this frame
	[[nodiscard]] virtual bool NeedsRedraw() const = 0;

	/// Check if UI wants to capture mouse input (mouse over a widget)
	[[nodiscard]] virtual bool WantsMouse() const = 0;

	// Factory - creates platform-appropriate backend
	static std::shared_ptr<GUIModule> CreateModule(
		std::shared_ptr<InputModule>&,
		std::shared_ptr<WindowModule>&,
		std::shared_ptr<RenderGraphModule>&);
};

// Default factory implementation returns nullptr
// Backend implementations should be used directly via the App template
inline std::shared_ptr<GUIModule> GUIModule::CreateModule(
	std::shared_ptr<InputModule>& inputModule,
	std::shared_ptr<WindowModule>& windowModule,
	std::shared_ptr<RenderGraphModule>& renderGraphModule)
{
	(void)inputModule;
	(void)windowModule;
	(void)renderGraphModule;
	return nullptr;
}
