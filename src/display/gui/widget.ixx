export module synodic.soul.gui:widget;

import std;
import synodic.soul.core;
import :ui_types;
import :ui_state;
import :draw_list;

// ============================================================================
// Widget Flags
// ============================================================================

export enum class WidgetFlags : std::uint32_t {
	None           = 0,
	Dirty          = 1 << 0,   // Needs re-layout
	Visible        = 1 << 1,   // Is rendered
	Enabled        = 1 << 2,   // Can receive input
	Focusable      = 1 << 3,   // Can receive keyboard focus
	Hovered        = 1 << 4,   // Mouse is over widget
	Focused        = 1 << 5,   // Has keyboard focus
	Pressed        = 1 << 6,   // Mouse button down on widget
	Interactive    = 1 << 7,   // Responds to input (for hit-testing)
};

export constexpr WidgetFlags operator|(WidgetFlags a, WidgetFlags b) {
	return static_cast<WidgetFlags>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

export constexpr WidgetFlags operator&(WidgetFlags a, WidgetFlags b) {
	return static_cast<WidgetFlags>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

export constexpr WidgetFlags operator~(WidgetFlags a) {
	return static_cast<WidgetFlags>(~static_cast<std::uint32_t>(a));
}

export constexpr bool HasFlag(WidgetFlags flags, WidgetFlags flag) {
	return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(flag)) != 0;
}

// ============================================================================
// Widget Base Class
// ============================================================================

export class Widget : public Component {

public:
	explicit Widget(WidgetID id = InvalidWidgetID)
		: id_(id == InvalidWidgetID ? GenerateId() : id)
		, flags_(WidgetFlags::Visible | WidgetFlags::Enabled) {}

	virtual ~Widget() = default;

	Widget(const Widget&) = delete;
	Widget(Widget&&) noexcept = default;
	Widget& operator=(const Widget&) = delete;
	Widget& operator=(Widget&&) noexcept = default;

	// =========================================================================
	// Identity
	// =========================================================================

	[[nodiscard]] WidgetID Id() const { return id_; }

	/// Set a path-based ID (for config binding persistence)
	void SetIdFromPath(std::string_view path) {
		id_ = HashWidgetPath(path);
	}

	// =========================================================================
	// Geometry
	// =========================================================================

	[[nodiscard]] const UIRect& Bounds() const { return bounds_; }
	[[nodiscard]] UIVec2 Position() const { return bounds_.position; }
	[[nodiscard]] UIVec2 Size() const { return bounds_.size; }

	void SetPosition(const UIVec2& position) {
		if (bounds_.position == position) return;
		bounds_.position = position;
		MarkDirty();
	}

	void SetSize(const UIVec2& size) {
		if (bounds_.size == size) return;
		bounds_.size = size;
		MarkDirty();
	}

	void SetBounds(const UIRect& bounds) {
		if (bounds_ == bounds) return;
		bounds_ = bounds;
		MarkDirty();
	}

	// =========================================================================
	// Z-Order (for hit-test priority)
	// =========================================================================

	[[nodiscard]] std::uint32_t ZIndex() const { return zIndex_; }
	void SetZIndex(std::uint32_t z) { zIndex_ = z; }

	// =========================================================================
	// Flags
	// =========================================================================

	[[nodiscard]] WidgetFlags Flags() const { return flags_; }

	[[nodiscard]] bool IsDirty() const { return HasFlag(flags_, WidgetFlags::Dirty); }
	[[nodiscard]] bool IsVisible() const { return HasFlag(flags_, WidgetFlags::Visible); }
	[[nodiscard]] bool IsEnabled() const { return HasFlag(flags_, WidgetFlags::Enabled); }
	[[nodiscard]] bool IsFocusable() const { return HasFlag(flags_, WidgetFlags::Focusable); }
	[[nodiscard]] bool IsHovered() const { return HasFlag(flags_, WidgetFlags::Hovered); }
	[[nodiscard]] bool IsFocused() const { return HasFlag(flags_, WidgetFlags::Focused); }
	[[nodiscard]] bool IsPressed() const { return HasFlag(flags_, WidgetFlags::Pressed); }
	[[nodiscard]] bool IsInteractive() const { return HasFlag(flags_, WidgetFlags::Interactive); }

	void SetVisible(bool visible) { SetFlag(WidgetFlags::Visible, visible); }
	void SetEnabled(bool enabled) { SetFlag(WidgetFlags::Enabled, enabled); }
	void SetFocusable(bool focusable) { SetFlag(WidgetFlags::Focusable, focusable); }
	void SetInteractive(bool interactive) { SetFlag(WidgetFlags::Interactive, interactive); }

	void MarkDirty() { SetFlag(WidgetFlags::Dirty, true); }
	void ClearDirty() { SetFlag(WidgetFlags::Dirty, false); }

	// =========================================================================
	// Parent/Child Hierarchy
	// =========================================================================

	[[nodiscard]] Widget* Parent() const { return parent_; }

	void SetParent(Widget* parent) { parent_ = parent; }

	/// Convert a point from screen space to widget-local space
	[[nodiscard]] UIVec2 ToLocal(const UIVec2& screenPos) const {
		return screenPos - bounds_.position;
	}

	/// Convert a point from widget-local space to screen space
	[[nodiscard]] UIVec2 ToScreen(const UIVec2& localPos) const {
		return localPos + bounds_.position;
	}

	// =========================================================================
	// Event Handling (virtual - override in derived classes)
	// =========================================================================

	/// Handle an input event. Return true if consumed.
	virtual bool OnEvent(UIEvent& event) {
		(void)event;
		return false;
	}

	/// Called when mouse enters the widget
	virtual void OnMouseEnter() {
		SetFlag(WidgetFlags::Hovered, true);
	}

	/// Called when mouse leaves the widget
	virtual void OnMouseLeave() {
		SetFlag(WidgetFlags::Hovered, false);
		SetFlag(WidgetFlags::Pressed, false);
	}

	/// Called when widget gains focus
	virtual void OnFocus() {
		SetFlag(WidgetFlags::Focused, true);
	}

	/// Called when widget loses focus
	virtual void OnBlur() {
		SetFlag(WidgetFlags::Focused, false);
	}

	// =========================================================================
	// State Persistence
	// =========================================================================

	/// Save widget state to store (for serialization)
	virtual void SaveState(WidgetState& state) const {
		state.Set("visible", IsVisible());
		state.Set("enabled", IsEnabled());
	}

	/// Load widget state from store
	virtual void LoadState(const WidgetState& state) {
		if (auto visible = state.Get<bool>("visible")) SetVisible(*visible);
		if (auto enabled = state.Get<bool>("enabled")) SetEnabled(*enabled);
	}

	// =========================================================================
	// Layout (virtual - override for custom layout)
	// =========================================================================

	/// Measure desired size (called during layout pass)
	virtual UIVec2 Measure(const UIVec2& availableSize) {
		(void)availableSize;
		return bounds_.size;
	}

	/// Arrange children within given bounds (called during layout pass)
	virtual void Arrange(const UIRect& finalBounds) {
		bounds_ = finalBounds;
		ClearDirty();
	}

	// =========================================================================
	// Drawing (virtual - override to emit draw commands)
	// =========================================================================

	/// Emit draw commands to the draw list
	/// Override in derived classes to render widget visuals
	virtual void Draw(UIDrawList& drawList) const {
		// Base widget draws nothing - override in derived classes
		(void)drawList;
	}

protected:
	void SetFlag(WidgetFlags flag, bool value) {
		if (value) {
			flags_ = flags_ | flag;
		} else {
			flags_ = flags_ & ~flag;
		}
	}

	// Internal state change hooks for derived classes
	virtual void OnHoveredChanged(bool hovered) { (void)hovered; }
	virtual void OnFocusedChanged(bool focused) { (void)focused; }
	virtual void OnPressedChanged(bool pressed) { (void)pressed; }

private:
	static WidgetID GenerateId() {
		static std::atomic<WidgetID> nextId{1};
		return nextId.fetch_add(1, std::memory_order_relaxed);
	}

	WidgetID id_;
	UIRect bounds_;
	std::uint32_t zIndex_ = 0;
	WidgetFlags flags_;
	Widget* parent_ = nullptr;
};
