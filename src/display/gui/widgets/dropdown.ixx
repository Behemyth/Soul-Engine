export module synodic.soul.gui:dropdown;

import std;
import :ui_types;
import :ui_state;
import :draw_list;
import :widget;

// ============================================================================
// Dropdown Option
// ============================================================================

export template<typename T>
struct DropdownOption {
	std::string label;
	T value;
	bool enabled = true;
};

// ============================================================================
// Dropdown Widget
// A retained-mode dropdown/combobox widget with typed values
// ============================================================================

export template<typename T>
class Dropdown : public Widget {
public:
	using Option = DropdownOption<T>;
	using SelectionCallback = std::function<void(std::size_t index, const T& value)>;

	explicit Dropdown(WidgetID id = InvalidWidgetID)
		: Widget(id) {
		SetInteractive(true);
		SetFocusable(true);
	}

	Dropdown(std::initializer_list<Option> options, WidgetID id = InvalidWidgetID)
		: Dropdown(id) {
		for (const auto& opt : options) {
			AddOption(opt.label, opt.value, opt.enabled);
		}
	}

	// =========================================================================
	// Options Management
	// =========================================================================

	/// Add an option to the dropdown
	void AddOption(std::string label, T value, bool enabled = true) {
		options_.push_back({std::move(label), std::move(value), enabled});
		MarkDirty();
	}

	/// Add multiple options from initializer list
	void AddOptions(std::initializer_list<std::pair<std::string, T>> options) {
		for (const auto& [label, value] : options) {
			AddOption(label, value);
		}
	}

	/// Clear all options
	void ClearOptions() {
		options_.clear();
		selectedIndex_ = std::nullopt;
		MarkDirty();
	}

	/// Get all options
	[[nodiscard]] const std::vector<Option>& Options() const { return options_; }

	/// Get option count
	[[nodiscard]] std::size_t OptionCount() const { return options_.size(); }

	/// Enable/disable a specific option
	void SetOptionEnabled(std::size_t index, bool enabled) {
		if (index < options_.size()) {
			options_[index].enabled = enabled;
			MarkDirty();
		}
	}

	// =========================================================================
	// Selection
	// =========================================================================

	/// Get currently selected index (nullopt if none)
	[[nodiscard]] std::optional<std::size_t> SelectedIndex() const { return selectedIndex_; }

	/// Get currently selected value (nullopt if none)
	[[nodiscard]] std::optional<T> SelectedValue() const {
		if (selectedIndex_ && *selectedIndex_ < options_.size()) {
			return options_[*selectedIndex_].value;
		}
		return std::nullopt;
	}

	/// Get currently selected label (empty if none)
	[[nodiscard]] std::string_view SelectedLabel() const {
		if (selectedIndex_ && *selectedIndex_ < options_.size()) {
			return options_[*selectedIndex_].label;
		}
		return placeholderText_;
	}

	/// Set selection by index
	void SetSelectedIndex(std::optional<std::size_t> index) {
		if (index == selectedIndex_) return;
		if (index && *index >= options_.size()) return;
		
		selectedIndex_ = index;
		MarkDirty();
		NotifySelectionChanged();
	}

	/// Set selection by value (selects first matching option)
	void SetSelectedValue(const T& value) {
		for (std::size_t i = 0; i < options_.size(); ++i) {
			if (options_[i].value == value) {
				SetSelectedIndex(i);
				return;
			}
		}
	}

	/// Clear selection
	void ClearSelection() {
		SetSelectedIndex(std::nullopt);
	}

	// =========================================================================
	// Placeholder
	// =========================================================================

	void SetPlaceholder(std::string text) {
		placeholderText_ = std::move(text);
		MarkDirty();
	}

	[[nodiscard]] std::string_view Placeholder() const { return placeholderText_; }

	// =========================================================================
	// Expanded State (open/closed)
	// =========================================================================

	[[nodiscard]] bool IsExpanded() const { return isExpanded_; }

	void SetExpanded(bool expanded) {
		if (isExpanded_ == expanded) return;
		isExpanded_ = expanded;
		MarkDirty();
	}

	void ToggleExpanded() {
		SetExpanded(!isExpanded_);
	}

	// =========================================================================
	// Dropdown List Bounds (for hit-testing when expanded)
	// =========================================================================

	[[nodiscard]] UIRect ExpandedBounds() const {
		if (!isExpanded_ || options_.empty()) {
			return Bounds();
		}
		
		UIRect result = Bounds();
		result.size.y += static_cast<double>(options_.size()) * optionHeight_;
		return result;
	}

	/// Get bounds of a specific option (relative to widget)
	[[nodiscard]] UIRect OptionBounds(std::size_t index) const {
		if (index >= options_.size()) return {};
		
		return {
			{0, Bounds().size.y + static_cast<double>(index) * optionHeight_},
			{Bounds().size.x, optionHeight_}
		};
	}

	/// Hit test which option is at a local position (-1 if none)
	[[nodiscard]] std::optional<std::size_t> HitTestOption(const UIVec2& localPos) const {
		if (!isExpanded_) return std::nullopt;
		
		// Check if in dropdown list area
		double listTop = Bounds().size.y;
		double listBottom = listTop + static_cast<double>(options_.size()) * optionHeight_;
		
		if (localPos.y >= listTop && localPos.y < listBottom) {
			auto index = static_cast<std::size_t>((localPos.y - listTop) / optionHeight_);
			if (index < options_.size()) {
				return index;
			}
		}
		return std::nullopt;
	}

	// =========================================================================
	// Styling (minimal for now, extensible later)
	// =========================================================================

	void SetOptionHeight(double height) { optionHeight_ = height; }
	[[nodiscard]] double OptionHeight() const { return optionHeight_; }

	// =========================================================================
	// Callbacks
	// =========================================================================

	/// Add a callback for selection changes
	void OnSelectionChanged(SelectionCallback callback) {
		selectionCallbacks_.push_back(std::move(callback));
	}

	// =========================================================================
	// Value Binding (for config integration)
	// =========================================================================

	/// Bind to an external value (two-way binding)
	void Bind(UIBinding<T> binding) {
		binding_ = std::move(binding);
		
		// Sync initial value from binding
		if (binding_.IsValid()) {
			SetSelectedValue(binding_.Get());
		}
	}

	/// Bind to a variable reference
	void BindTo(T& variable) {
		Bind(UIBinding<T>::ToVariable(variable));
	}

	// =========================================================================
	// Event Handling
	// =========================================================================

	bool OnEvent(UIEvent& event) override {
		if (!IsEnabled()) return false;

		switch (event.type) {
			case UIEventType::MouseDown:
				if (event.button == MouseButton::Left) {
					SetFlag(WidgetFlags::Pressed, true);
					return true;
				}
				break;

			case UIEventType::MouseUp:
			case UIEventType::Click:
				if (event.button == MouseButton::Left) {
					SetFlag(WidgetFlags::Pressed, false);
					HandleClick(event.position);
					return true;
				}
				break;

			case UIEventType::KeyDown:
				return HandleKeyDown(event.keyCode);

			default:
				break;
		}

		return false;
	}

	void OnBlur() override {
		Widget::OnBlur();
		// Close dropdown when losing focus
		SetExpanded(false);
	}

	// =========================================================================
	// State Persistence
	// =========================================================================

	void SaveState(WidgetState& state) const override {
		Widget::SaveState(state);
		
		if (selectedIndex_) {
			state.Set("selectedIndex", static_cast<std::int64_t>(*selectedIndex_));
		}
	}

	void LoadState(const WidgetState& state) override {
		Widget::LoadState(state);
		
		if (auto index = state.Get<std::int64_t>("selectedIndex")) {
			SetSelectedIndex(static_cast<std::size_t>(*index));
		}
	}

	// =========================================================================
	// Layout
	// =========================================================================

	UIVec2 Measure(const UIVec2& availableSize) override {
		// Default size based on content
		double maxWidth = 100.0;  // Minimum width
		for (const auto& opt : options_) {
			// Approximate text width (would use font metrics in full impl)
			maxWidth = std::max(maxWidth, static_cast<double>(opt.label.size()) * 8.0 + 40.0);
		}
		
		// Clamp to available size
		double width = std::min(maxWidth, availableSize.x);
		double height = optionHeight_;  // Collapsed height
		
		return {width, height};
	}

	// =========================================================================
	// Drawing
	// =========================================================================

	void Draw(UIDrawList& drawList) const override {
		// Colors
		constexpr std::uint32_t bgNormal = 0x3A3A3AFF;
		constexpr std::uint32_t bgHovered = 0x4A4A4AFF;
		constexpr std::uint32_t bgPressed = 0x2A2A2AFF;
		constexpr std::uint32_t bgExpanded = 0x454545FF;
		constexpr std::uint32_t optionHovered = 0x5A5A5AFF;
		constexpr std::uint32_t textColor = 0xFFFFFFFF;
		constexpr std::uint32_t textDisabled = 0x808080FF;
		constexpr std::uint32_t borderColor = 0x5A5A5AFF;

		// Determine button background color based on state
		std::uint32_t buttonBg = bgNormal;
		if (IsPressed()) {
			buttonBg = bgPressed;
		} else if (IsHovered() || isExpanded_) {
			buttonBg = bgHovered;
		}

		// Draw main button
		drawList.AddBorderedRect(Bounds(), buttonBg, borderColor, 1.0f, 3.0f);

		// Draw selected text or placeholder
		UIRect textBounds = Bounds();
		textBounds.position.x += 8.0;  // Left padding
		textBounds.size.x -= 24.0;     // Leave room for arrow
		drawList.AddText(textBounds, SelectedLabel(), 
		                 selectedIndex_ ? textColor : textDisabled);

		// Draw dropdown arrow (simple triangle approximation as text)
		UIRect arrowBounds = {{Bounds().Right() - 20.0, Bounds().position.y}, {16.0, Bounds().size.y}};
		drawList.AddText(arrowBounds, isExpanded_ ? "^" : "v", textColor);

		// Draw expanded dropdown list
		if (isExpanded_ && !options_.empty()) {
			// Use scissor to allow dropdown to extend beyond parent bounds
			UIRect listBounds = {
				{Bounds().position.x, Bounds().Bottom()},
				{Bounds().size.x, static_cast<double>(options_.size()) * optionHeight_}
			};

			// Dropdown list background
			drawList.AddBorderedRect(listBounds, bgExpanded, borderColor, 1.0f, 3.0f);

			// Draw each option
			for (std::size_t i = 0; i < options_.size(); ++i) {
				const auto& opt = options_[i];
				UIRect optRect = {
					{Bounds().position.x, Bounds().Bottom() + static_cast<double>(i) * optionHeight_},
					{Bounds().size.x, optionHeight_}
				};

				// Option hover highlight
				if (hoveredOption_ == i) {
					drawList.AddRect(optRect, optionHovered);
				}

				// Option text
				UIRect optTextBounds = optRect;
				optTextBounds.position.x += 8.0;
				optTextBounds.size.x -= 16.0;
				drawList.AddText(optTextBounds, opt.label, 
				                 opt.enabled ? textColor : textDisabled);
			}
		}
	}

private:
	void HandleClick(const UIVec2& localPos) {
		// Check if click is in main button area
		if (localPos.y < Bounds().size.y) {
			ToggleExpanded();
			return;
		}

		// Check if click is in an option
		if (isExpanded_) {
			if (auto optIndex = HitTestOption(localPos)) {
				if (options_[*optIndex].enabled) {
					SetSelectedIndex(*optIndex);
					SetExpanded(false);
				}
			}
		}
	}

	bool HandleKeyDown(std::uint32_t keyCode) {
		// Basic keyboard navigation
		// Key codes would be SDL scancodes or similar
		constexpr std::uint32_t KEY_UP = 82;     // SDL_SCANCODE_UP
		constexpr std::uint32_t KEY_DOWN = 81;   // SDL_SCANCODE_DOWN
		constexpr std::uint32_t KEY_ENTER = 40;  // SDL_SCANCODE_RETURN
		constexpr std::uint32_t KEY_ESCAPE = 41; // SDL_SCANCODE_ESCAPE

		if (keyCode == KEY_ESCAPE) {
			SetExpanded(false);
			return true;
		}

		if (keyCode == KEY_ENTER) {
			if (isExpanded_ && hoveredOption_) {
				if (options_[*hoveredOption_].enabled) {
					SetSelectedIndex(*hoveredOption_);
					SetExpanded(false);
				}
			} else {
				ToggleExpanded();
			}
			return true;
		}

		if (isExpanded_) {
			if (keyCode == KEY_DOWN) {
				NavigateOptions(1);
				return true;
			}
			if (keyCode == KEY_UP) {
				NavigateOptions(-1);
				return true;
			}
		}

		return false;
	}

	void NavigateOptions(int delta) {
		if (options_.empty()) return;

		std::size_t current = hoveredOption_.value_or(
			selectedIndex_.value_or(delta > 0 ? options_.size() - 1 : 0)
		);

		// Find next enabled option
		for (std::size_t i = 0; i < options_.size(); ++i) {
			if (delta > 0) {
				current = (current + 1) % options_.size();
			} else {
				current = (current + options_.size() - 1) % options_.size();
			}

			if (options_[current].enabled) {
				hoveredOption_ = current;
				MarkDirty();
				return;
			}
		}
	}

	void NotifySelectionChanged() {
		if (!selectedIndex_) return;

		const T& value = options_[*selectedIndex_].value;

		// Update binding
		if (binding_.IsValid() && !binding_.IsReadOnly()) {
			binding_.Set(value);
		}

		// Notify callbacks
		for (const auto& callback : selectionCallbacks_) {
			callback(*selectedIndex_, value);
		}
	}

	std::vector<Option> options_;
	std::optional<std::size_t> selectedIndex_;
	std::optional<std::size_t> hoveredOption_;
	std::string placeholderText_ = "Select...";
	bool isExpanded_ = false;
	double optionHeight_ = 24.0;

	UIBinding<T> binding_;
	std::vector<SelectionCallback> selectionCallbacks_;
};
