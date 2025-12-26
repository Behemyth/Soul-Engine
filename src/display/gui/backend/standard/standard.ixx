export module synodic.soul.gui.backend.standard;

import std;
import synodic.soul.gui;

// ============================================================================
// Standard GUI Backend
// Manages widget tree, hit-testing, event dispatch, and draw list generation
// Integrates with render graph via UIDrawList output
// ============================================================================

export class StandardGUIBackend : public GUIModule {
public:
	StandardGUIBackend() = default;
	~StandardGUIBackend() override = default;

	StandardGUIBackend(const StandardGUIBackend&) = delete;
	StandardGUIBackend(StandardGUIBackend&&) noexcept = default;
	StandardGUIBackend& operator=(const StandardGUIBackend&) = delete;
	StandardGUIBackend& operator=(StandardGUIBackend&&) noexcept = default;

	// =========================================================================
	// GUIModule Interface
	// =========================================================================

	void Update(std::chrono::nanoseconds deltaTime) override {
		(void)deltaTime;

		// Rebuild spatial index if any widget is dirty
		if (needsSpatialRebuild_) {
			RebuildSpatialIndex();
			needsSpatialRebuild_ = false;
		}

		// Run layout pass if needed
		if (root_.IsDirty()) {
			PerformLayout();
		}

		// Generate draw commands if UI changed
		if (needsRedraw_) {
			GenerateDrawList();
			needsRedraw_ = false;
		}
	}

	[[nodiscard]] const UIDrawList& GetDrawList() const override {
		return drawList_;
	}

	[[nodiscard]] UIRenderData GetRenderData() const override {
		UIRenderData data;
		data.drawList = &drawList_;
		data.viewportWidth = static_cast<float>(viewportSize_.x);
		data.viewportHeight = static_cast<float>(viewportSize_.y);
		data.fontAtlasTexture = fontAtlasTexture_;
		data.scaleX = 1.0f;
		data.scaleY = 1.0f;
		return data;
	}

	[[nodiscard]] bool NeedsRedraw() const override {
		return needsRedraw_;
	}

	/// Check if mouse is over any widget (UI should capture input)
	[[nodiscard]] bool WantsMouse() const override {
		return hoveredWidget_ != nullptr || pressedWidget_ != nullptr;
	}

	// =========================================================================
	// Root Layout Access
	// =========================================================================

	[[nodiscard]] Layout& Root() { return root_; }
	[[nodiscard]] const Layout& Root() const { return root_; }

	// =========================================================================
	// State Management
	// =========================================================================

	[[nodiscard]] UIStateStore& StateStore() { return stateStore_; }
	[[nodiscard]] const UIStateStore& StateStore() const { return stateStore_; }

	/// Save all widget states to the store
	void SaveAllStates() {
		SaveWidgetState(&root_);
	}

	/// Load all widget states from the store
	void LoadAllStates() {
		LoadWidgetState(&root_);
	}

	// =========================================================================
	// Hit Testing
	// =========================================================================

	/// Perform hit test at screen position
	[[nodiscard]] HitTestResult HitTest(const UIVec2& screenPos) const {
		HitTestResult result;
		
		// Query spatial index for candidates
		std::vector<WidgetID> candidates;
		spatialIndex_.QueryPoint(screenPos, candidates);

		if (candidates.empty()) {
			return result;
		}

		// Find the top-most (highest z-index) widget
		std::uint32_t maxZ = 0;
		Widget* best = nullptr;

		for (WidgetID id : candidates) {
			if (Widget* widget = const_cast<Layout&>(root_).FindWidget(id)) {
				if (!widget->IsVisible() || !widget->IsEnabled()) continue;

				if (widget->ZIndex() >= maxZ) {
					maxZ = widget->ZIndex();
					best = widget;
				}
			}
		}

		if (best) {
			result.widgetId = best->Id();
			result.widget = best;
			result.localPosition = best->ToLocal(screenPos);
			result.depth = maxZ;
		}

		return result;
	}

	/// Collect all widgets at a position (for debugging/inspection)
	std::vector<HitTestResult> HitTestAll(const UIVec2& screenPos) const {
		std::vector<HitTestResult> results;
		
		std::vector<WidgetID> candidates;
		spatialIndex_.QueryPoint(screenPos, candidates);

		for (WidgetID id : candidates) {
			if (Widget* widget = const_cast<Layout&>(root_).FindWidget(id)) {
				if (!widget->IsVisible()) continue;

				HitTestResult hit;
				hit.widgetId = id;
				hit.widget = widget;
				hit.localPosition = widget->ToLocal(screenPos);
				hit.depth = widget->ZIndex();
				results.push_back(hit);
			}
		}

		// Sort by z-index descending (front to back)
		std::sort(results.begin(), results.end(),
			[](const auto& a, const auto& b) { return a.depth > b.depth; });

		return results;
	}

	// =========================================================================
	// Input Event Processing
	// =========================================================================

	/// Process a mouse move event
	void OnMouseMove(const UIVec2& screenPos) {
		currentMousePos_ = screenPos;

		// Hit test for hover
		auto hit = HitTest(screenPos);
		
		// Handle hover state changes
		if (hit.widget != hoveredWidget_) {
			if (hoveredWidget_) {
				hoveredWidget_->OnMouseLeave();
				needsRedraw_ = true;
			}
			hoveredWidget_ = hit.widget;
			if (hoveredWidget_) {
				hoveredWidget_->OnMouseEnter();
				needsRedraw_ = true;
			}
		}

		// Dispatch move event to hovered widget
		if (hoveredWidget_) {
			UIEvent event;
			event.type = UIEventType::MouseMove;
			event.globalPosition = screenPos;
			event.position = hit.localPosition;
			hoveredWidget_->OnEvent(event);
		}
	}

	/// Process a mouse button event
	void OnMouseButton(MouseButton button, bool pressed, const UIVec2& screenPos) {
		auto hit = HitTest(screenPos);

		UIEvent event;
		event.type = pressed ? UIEventType::MouseDown : UIEventType::MouseUp;
		event.button = button;
		event.globalPosition = screenPos;

		if (hit.widget) {
			event.position = hit.localPosition;
			
			if (pressed) {
				// Set focus to clicked widget
				if (focusedWidget_ != hit.widget) {
					if (focusedWidget_) {
						focusedWidget_->OnBlur();
					}
					focusedWidget_ = hit.widget->IsFocusable() ? hit.widget : nullptr;
					if (focusedWidget_) {
						focusedWidget_->OnFocus();
					}
					needsRedraw_ = true;
				}

				pressedWidget_ = hit.widget;
			}
			
			if (hit.widget->OnEvent(event)) {
				needsRedraw_ = true;
			}

			// Generate click event on release within same widget
			if (!pressed && pressedWidget_ == hit.widget) {
				UIEvent clickEvent;
				clickEvent.type = UIEventType::Click;
				clickEvent.button = button;
				clickEvent.globalPosition = screenPos;
				clickEvent.position = hit.localPosition;
				if (hit.widget->OnEvent(clickEvent)) {
					needsRedraw_ = true;
				}
			}
		} else if (pressed) {
			// Click outside any widget - clear focus
			if (focusedWidget_) {
				focusedWidget_->OnBlur();
				focusedWidget_ = nullptr;
				needsRedraw_ = true;
			}
		}

		if (!pressed) {
			pressedWidget_ = nullptr;
		}
	}

	/// Process a key event
	void OnKey(std::uint32_t keyCode, bool pressed) {
		if (!focusedWidget_) return;

		UIEvent event;
		event.type = pressed ? UIEventType::KeyDown : UIEventType::KeyUp;
		event.keyCode = keyCode;
		event.globalPosition = currentMousePos_;
		event.position = focusedWidget_->ToLocal(currentMousePos_);

		if (focusedWidget_->OnEvent(event)) {
			needsRedraw_ = true;
		}
	}

	// =========================================================================
	// Viewport Configuration
	// =========================================================================

	void SetViewportSize(const UIVec2& size) {
		if (viewportSize_ == size) return;
		viewportSize_ = size;
		root_.SetSize(size);
		needsSpatialRebuild_ = true;
		needsRedraw_ = true;
	}

	[[nodiscard]] UIVec2 ViewportSize() const { return viewportSize_; }

	// =========================================================================
	// Font Atlas (for text rendering)
	// =========================================================================

	void SetFontAtlasTexture(std::uint32_t handle) {
		fontAtlasTexture_ = handle;
	}

	// =========================================================================
	// Spatial Index Management
	// =========================================================================

	/// Force rebuild of spatial index
	void InvalidateSpatialIndex() {
		needsSpatialRebuild_ = true;
	}

	/// Mark UI as needing redraw
	void InvalidateDrawList() {
		needsRedraw_ = true;
	}

private:
	void PerformLayout() {
		// Measure pass
		root_.Measure(viewportSize_);

		// Arrange pass
		UIRect rootBounds = {{0, 0}, viewportSize_};
		root_.Arrange(rootBounds);

		// Spatial index needs update after layout
		needsSpatialRebuild_ = true;
		needsRedraw_ = true;
	}

	void RebuildSpatialIndex() {
		spatialIndex_.Clear();
		root_.RegisterWithSpatialIndex(spatialIndex_);
	}

	void GenerateDrawList() {
		drawList_.Clear();
		DrawWidget(&root_);
	}

	void DrawWidget(Widget* widget) {
		if (!widget->IsVisible()) return;

		// Draw the widget itself
		widget->Draw(drawList_);

		// Recurse into layouts
		if (auto* layout = dynamic_cast<Layout*>(widget)) {
			for (auto& child : *layout) {
				DrawWidget(child.get());
			}
		}
	}

	void SaveWidgetState(Widget* widget) {
		auto& state = stateStore_.GetState(widget->Id());
		widget->SaveState(state);

		// Recurse for layouts
		if (auto* layout = dynamic_cast<Layout*>(widget)) {
			for (auto& child : *layout) {
				SaveWidgetState(child.get());
			}
		}
	}

	void LoadWidgetState(Widget* widget) {
		if (stateStore_.HasState(widget->Id())) {
			widget->LoadState(stateStore_.GetState(widget->Id()));
		}

		// Recurse for layouts
		if (auto* layout = dynamic_cast<Layout*>(widget)) {
			for (auto& child : *layout) {
				LoadWidgetState(child.get());
			}
		}
	}

	// Widget tree
	Layout root_;

	// Draw list for render graph
	UIDrawList drawList_;
	bool needsRedraw_ = true;

	// Spatial indexing for efficient hit-testing
	LinearSpatialIndex spatialIndex_;  // Start simple, upgrade to QuadTree when needed
	bool needsSpatialRebuild_ = true;

	// State persistence
	UIStateStore stateStore_;

	// Input state tracking
	Widget* hoveredWidget_ = nullptr;
	Widget* focusedWidget_ = nullptr;
	Widget* pressedWidget_ = nullptr;
	UIVec2 currentMousePos_;

	// Viewport
	UIVec2 viewportSize_ = {1920, 1080};

	// Font atlas texture handle
	std::uint32_t fontAtlasTexture_ = 0;
};
