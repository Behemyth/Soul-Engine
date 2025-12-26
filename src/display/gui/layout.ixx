export module synodic.soul.gui:layout;

import std;
import :ui_types;
import :ui_state;
import :draw_list;
import :widget;

// ============================================================================
// Layout Direction
// ============================================================================

export enum class LayoutDirection {
	Vertical,    // Stack children top-to-bottom
	Horizontal   // Stack children left-to-right
};

// ============================================================================
// Layout Alignment
// ============================================================================

export enum class LayoutAlign {
	Start,    // Left/Top
	Center,
	End,      // Right/Bottom
	Stretch   // Fill available space
};

// ============================================================================
// Layout Parameters
// ============================================================================

export struct LayoutParams {
	LayoutDirection direction = LayoutDirection::Vertical;
	LayoutAlign mainAlign = LayoutAlign::Start;      // Along direction axis
	LayoutAlign crossAlign = LayoutAlign::Stretch;   // Perpendicular to direction
	double spacing = 4.0;                            // Gap between children
	UIVec2 padding = {8.0, 8.0};                    // Internal padding
};

// ============================================================================
// Layout Container
// ============================================================================

export class Layout : public Widget {

public:
	explicit Layout(WidgetID id = InvalidWidgetID)
		: Widget(id) {}

	explicit Layout(const LayoutParams& params, WidgetID id = InvalidWidgetID)
		: Widget(id), params_(params) {}

	~Layout() override = default;

	Layout(const Layout&) = delete;
	Layout(Layout&&) noexcept = default;
	Layout& operator=(const Layout&) = delete;
	Layout& operator=(Layout&&) noexcept = default;

	// =========================================================================
	// Layout Parameters
	// =========================================================================

	[[nodiscard]] const LayoutParams& Params() const { return params_; }

	void SetParams(const LayoutParams& params) {
		params_ = params;
		MarkDirty();
	}

	void SetDirection(LayoutDirection dir) {
		params_.direction = dir;
		MarkDirty();
	}

	void SetSpacing(double spacing) {
		params_.spacing = spacing;
		MarkDirty();
	}

	void SetPadding(const UIVec2& padding) {
		params_.padding = padding;
		MarkDirty();
	}

	// =========================================================================
	// Child Management
	// =========================================================================

	/// Add a child widget, returning a reference to it
	template<typename T, typename... Args>
	T& AddWidget(Args&&... args) {
		static_assert(std::is_base_of_v<Widget, T>, "T must derive from Widget");
		auto widget = std::make_unique<T>(std::forward<Args>(args)...);
		widget->SetParent(this);
		T& ref = *widget;
		children_.push_back(std::move(widget));
		MarkDirty();
		return ref;
	}

	/// Add a child layout, returning a reference to it
	template<typename T = Layout, typename... Args>
	T& AddLayout(Args&&... args) {
		return AddWidget<T>(std::forward<Args>(args)...);
	}

	/// Remove a child by pointer
	bool RemoveWidget(Widget* widget) {
		auto it = std::find_if(children_.begin(), children_.end(),
			[widget](const auto& ptr) { return ptr.get() == widget; });
		
		if (it != children_.end()) {
			children_.erase(it);
			MarkDirty();
			return true;
		}
		return false;
	}

	/// Remove a child by ID
	bool RemoveWidget(WidgetID id) {
		auto it = std::find_if(children_.begin(), children_.end(),
			[id](const auto& ptr) { return ptr->Id() == id; });
		
		if (it != children_.end()) {
			children_.erase(it);
			MarkDirty();
			return true;
		}
		return false;
	}

	/// Clear all children
	void ClearChildren() {
		children_.clear();
		MarkDirty();
	}

	/// Get child count
	[[nodiscard]] std::size_t ChildCount() const { return children_.size(); }

	/// Get child by index
	[[nodiscard]] Widget* Child(std::size_t index) const {
		return index < children_.size() ? children_[index].get() : nullptr;
	}

	/// Iterate over children
	[[nodiscard]] auto begin() { return children_.begin(); }
	[[nodiscard]] auto end() { return children_.end(); }
	[[nodiscard]] auto begin() const { return children_.begin(); }
	[[nodiscard]] auto end() const { return children_.end(); }

	// =========================================================================
	// Widget Lookup (for hit-testing support)
	// =========================================================================

	/// Find widget by ID in this layout and descendants
	Widget* FindWidget(WidgetID id) {
		for (auto& child : children_) {
			if (child->Id() == id) {
				return child.get();
			}
			// Recursively search layouts
			if (auto* layout = dynamic_cast<Layout*>(child.get())) {
				if (auto* found = layout->FindWidget(id)) {
					return found;
				}
			}
		}
		return nullptr;
	}

	/// Collect all interactive widgets recursively
	void CollectInteractiveWidgets(std::vector<Widget*>& out) {
		for (auto& child : children_) {
			if (child->IsInteractive() && child->IsVisible() && child->IsEnabled()) {
				out.push_back(child.get());
			}
			if (auto* layout = dynamic_cast<Layout*>(child.get())) {
				layout->CollectInteractiveWidgets(out);
			}
		}
	}

	/// Register all visible widgets with a spatial index
	void RegisterWithSpatialIndex(ISpatialIndex& index) {
		for (auto& child : children_) {
			if (child->IsVisible() && child->IsInteractive()) {
				index.Insert(child->Id(), child->Bounds());
			}
			if (auto* layout = dynamic_cast<Layout*>(child.get())) {
				layout->RegisterWithSpatialIndex(index);
			}
		}
	}

	// =========================================================================
	// Event Propagation
	// =========================================================================

	bool OnEvent(UIEvent& event) override {
		// Dispatch to children in reverse order (front-to-back)
		for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
			auto& child = *it;
			if (!child->IsVisible() || !child->IsEnabled()) continue;

			// Check if event position is within child bounds
			if (child->Bounds().Contains(event.globalPosition)) {
				// Transform to child-local coords
				UIEvent childEvent = event;
				childEvent.position = child->ToLocal(event.globalPosition);

				if (child->OnEvent(childEvent)) {
					event.consumed = true;
					return true;
				}
			}
		}
		return false;
	}

	// =========================================================================
	// Layout Algorithm
	// =========================================================================

	UIVec2 Measure(const UIVec2& availableSize) override {
		UIVec2 contentSize = {0, 0};
		UIVec2 innerAvailable = {
			availableSize.x - params_.padding.x * 2,
			availableSize.y - params_.padding.y * 2
		};

		bool isVertical = (params_.direction == LayoutDirection::Vertical);

		for (auto& child : children_) {
			if (!child->IsVisible()) continue;

			UIVec2 childSize = child->Measure(innerAvailable);

			if (isVertical) {
				contentSize.x = std::max(contentSize.x, childSize.x);
				contentSize.y += childSize.y + params_.spacing;
			} else {
				contentSize.x += childSize.x + params_.spacing;
				contentSize.y = std::max(contentSize.y, childSize.y);
			}
		}

		// Remove trailing spacing
		if (!children_.empty()) {
			if (isVertical) {
				contentSize.y -= params_.spacing;
			} else {
				contentSize.x -= params_.spacing;
			}
		}

		// Add padding
		return {
			contentSize.x + params_.padding.x * 2,
			contentSize.y + params_.padding.y * 2
		};
	}

	void Arrange(const UIRect& finalBounds) override {
		Widget::Arrange(finalBounds);

		bool isVertical = (params_.direction == LayoutDirection::Vertical);
		
		UIVec2 pos = {
			finalBounds.position.x + params_.padding.x,
			finalBounds.position.y + params_.padding.y
		};
		
		UIVec2 innerSize = {
			finalBounds.size.x - params_.padding.x * 2,
			finalBounds.size.y - params_.padding.y * 2
		};

		for (auto& child : children_) {
			if (!child->IsVisible()) continue;

			UIVec2 childSize = child->Measure(innerSize);
			UIRect childBounds;

			if (isVertical) {
				// Apply cross-axis alignment
				double x = pos.x;
				double width = childSize.x;

				if (params_.crossAlign == LayoutAlign::Stretch) {
					width = innerSize.x;
				} else if (params_.crossAlign == LayoutAlign::Center) {
					x = pos.x + (innerSize.x - childSize.x) * 0.5;
				} else if (params_.crossAlign == LayoutAlign::End) {
					x = pos.x + innerSize.x - childSize.x;
				}

				childBounds = {{x, pos.y}, {width, childSize.y}};
				pos.y += childSize.y + params_.spacing;
			} else {
				// Horizontal layout
				double y = pos.y;
				double height = childSize.y;

				if (params_.crossAlign == LayoutAlign::Stretch) {
					height = innerSize.y;
				} else if (params_.crossAlign == LayoutAlign::Center) {
					y = pos.y + (innerSize.y - childSize.y) * 0.5;
				} else if (params_.crossAlign == LayoutAlign::End) {
					y = pos.y + innerSize.y - childSize.y;
				}

				childBounds = {{pos.x, y}, {childSize.x, height}};
				pos.x += childSize.x + params_.spacing;
			}

			child->Arrange(childBounds);
		}
	}

	// =========================================================================
	// State Persistence
	// =========================================================================

	void SaveState(WidgetState& state) const override {
		Widget::SaveState(state);
		// Save child states recursively would be done at UIStateStore level
	}

	void LoadState(const WidgetState& state) override {
		Widget::LoadState(state);
	}

protected:
	std::vector<std::unique_ptr<Widget>> children_;
	LayoutParams params_;
};

// ============================================================================
// Panel - A layout with visual background (for rendering backends)
// ============================================================================

export class Panel : public Layout {
public:
	explicit Panel(WidgetID id = InvalidWidgetID) : Layout(id) {}
	explicit Panel(const LayoutParams& params, WidgetID id = InvalidWidgetID) 
		: Layout(params, id) {}

	// Panel-specific styling (minimal for now)
	void SetBackgroundVisible(bool visible) { hasBackground_ = visible; }
	[[nodiscard]] bool HasBackground() const { return hasBackground_; }

	void SetBackgroundColor(std::uint32_t color) { backgroundColor_ = color; }
	void SetBorderColor(std::uint32_t color) { borderColor_ = color; }
	void SetCornerRadius(float radius) { cornerRadius_ = radius; }

	// =========================================================================
	// Drawing
	// =========================================================================

	void Draw(UIDrawList& drawList) const override {
		if (!hasBackground_) return;

		// Draw panel background with rounded corners
		if (cornerRadius_ > 0.0f) {
			drawList.AddBorderedRect(Bounds(), backgroundColor_, borderColor_, 1.0f, cornerRadius_);
		} else {
			drawList.AddRect(Bounds(), backgroundColor_);
		}
	}

private:
	bool hasBackground_ = true;
	std::uint32_t backgroundColor_ = 0x2A2A2AE0;  // Semi-transparent dark gray
	std::uint32_t borderColor_ = 0x4A4A4AFF;      // Lighter gray border
	float cornerRadius_ = 4.0f;
};
