export module synodic.soul.gui:ui_types;

import std;

// ============================================================================
// Core UI Types
// ============================================================================

/// 2D position/size for UI elements (sub-pixel precision)
export struct UIVec2 {
	double x = 0.0;
	double y = 0.0;

	UIVec2 operator+(const UIVec2& other) const { return {x + other.x, y + other.y}; }
	UIVec2 operator-(const UIVec2& other) const { return {x - other.x, y - other.y}; }
	UIVec2 operator*(double scalar) const { return {x * scalar, y * scalar}; }
	bool operator==(const UIVec2&) const = default;
};

/// Axis-aligned bounding box for widgets
export struct UIRect {
	UIVec2 position;  // Upper-left corner
	UIVec2 size;

	[[nodiscard]] double Left() const { return position.x; }
	[[nodiscard]] double Right() const { return position.x + size.x; }
	[[nodiscard]] double Top() const { return position.y; }
	[[nodiscard]] double Bottom() const { return position.y + size.y; }
	[[nodiscard]] UIVec2 Center() const { return {position.x + size.x * 0.5, position.y + size.y * 0.5}; }

	[[nodiscard]] bool Contains(const UIVec2& point) const {
		return point.x >= Left() && point.x < Right() &&
		       point.y >= Top() && point.y < Bottom();
	}

	[[nodiscard]] bool Intersects(const UIRect& other) const {
		return Left() < other.Right() && Right() > other.Left() &&
		       Top() < other.Bottom() && Bottom() > other.Top();
	}

	bool operator==(const UIRect&) const = default;
};

/// Widget identifier - stable across frames for state persistence
export using WidgetID = std::uint64_t;
export constexpr WidgetID InvalidWidgetID = 0;

/// Generate a unique widget ID from a string path (for config binding)
export inline WidgetID HashWidgetPath(std::string_view path) {
	// FNV-1a hash
	std::uint64_t hash = 14695981039346656037ULL;
	for (char c : path) {
		hash ^= static_cast<std::uint64_t>(c);
		hash *= 1099511628211ULL;
	}
	return hash;
}

// ============================================================================
// UI Input Events
// ============================================================================

export enum class UIEventType {
	None,
	MouseEnter,
	MouseLeave,
	MouseMove,
	MouseDown,
	MouseUp,
	Click,
	Focus,
	Blur,
	KeyDown,
	KeyUp
};

export enum class MouseButton : std::uint8_t {
	None = 0,
	Left = 1,
	Middle = 2,
	Right = 3
};

export struct UIEvent {
	UIEventType type = UIEventType::None;
	UIVec2 position;           // Mouse position in widget-local coords
	UIVec2 globalPosition;     // Mouse position in screen coords
	MouseButton button = MouseButton::None;
	std::uint32_t keyCode = 0;
	bool consumed = false;     // Set true to stop propagation
};

// ============================================================================
// Hit Test Result
// ============================================================================

export struct HitTestResult {
	WidgetID widgetId = InvalidWidgetID;
	class Widget* widget = nullptr;
	UIVec2 localPosition;      // Position relative to widget
	std::uint32_t depth = 0;   // Z-order depth
};

// ============================================================================
// Spatial Index Interface (for hit-testing)
// ============================================================================

/// Abstract interface for spatial indexing strategies
/// Implementations: QuadTree, R-Tree, or simple linear scan for small UI
export class ISpatialIndex {
public:
	virtual ~ISpatialIndex() = default;

	/// Insert a widget into the spatial index
	virtual void Insert(WidgetID id, const UIRect& bounds) = 0;

	/// Remove a widget from the index
	virtual void Remove(WidgetID id) = 0;

	/// Update widget bounds (for animation/layout changes)
	virtual void Update(WidgetID id, const UIRect& newBounds) = 0;

	/// Clear all entries
	virtual void Clear() = 0;

	/// Query all widgets containing a point (returns in front-to-back order)
	virtual void QueryPoint(const UIVec2& point, std::vector<WidgetID>& results) const = 0;

	/// Query all widgets intersecting a rectangle
	virtual void QueryRect(const UIRect& rect, std::vector<WidgetID>& results) const = 0;
};

// ============================================================================
// Quadtree Implementation (future-proof for large UIs)
// ============================================================================

export class QuadTreeSpatialIndex final : public ISpatialIndex {
public:
	explicit QuadTreeSpatialIndex(const UIRect& bounds, std::uint32_t maxDepth = 8, std::uint32_t maxPerNode = 8)
		: bounds_(bounds), maxDepth_(maxDepth), maxPerNode_(maxPerNode) {
		root_ = std::make_unique<Node>();
	}

	void Insert(WidgetID id, const UIRect& bounds) override {
		entries_[id] = bounds;
		InsertIntoNode(root_.get(), bounds_, id, bounds, 0);
	}

	void Remove(WidgetID id) override {
		if (auto it = entries_.find(id); it != entries_.end()) {
			RemoveFromNode(root_.get(), bounds_, id, it->second, 0);
			entries_.erase(it);
		}
	}

	void Update(WidgetID id, const UIRect& newBounds) override {
		Remove(id);
		Insert(id, newBounds);
	}

	void Clear() override {
		root_ = std::make_unique<Node>();
		entries_.clear();
	}

	void QueryPoint(const UIVec2& point, std::vector<WidgetID>& results) const override {
		results.clear();
		QueryPointInNode(root_.get(), bounds_, point, results);
	}

	void QueryRect(const UIRect& rect, std::vector<WidgetID>& results) const override {
		results.clear();
		QueryRectInNode(root_.get(), bounds_, rect, results);
	}

private:
	struct Node {
		std::vector<WidgetID> widgets;
		std::array<std::unique_ptr<Node>, 4> children;  // NW, NE, SW, SE
		bool isLeaf = true;
	};

	void InsertIntoNode(Node* node, const UIRect& nodeBounds, WidgetID id, const UIRect& widgetBounds, std::uint32_t depth) {
		if (!nodeBounds.Intersects(widgetBounds)) return;

		if (node->isLeaf) {
			node->widgets.push_back(id);

			// Split if over capacity and not at max depth
			if (node->widgets.size() > maxPerNode_ && depth < maxDepth_) {
				SplitNode(node, nodeBounds, depth);
			}
		} else {
			auto childBounds = GetChildBounds(nodeBounds);
			for (std::size_t i = 0; i < 4; ++i) {
				if (node->children[i]) {
					InsertIntoNode(node->children[i].get(), childBounds[i], id, widgetBounds, depth + 1);
				}
			}
		}
	}

	void RemoveFromNode(Node* node, const UIRect& nodeBounds, WidgetID id, const UIRect& widgetBounds, std::uint32_t depth) {
		if (!nodeBounds.Intersects(widgetBounds)) return;

		if (node->isLeaf) {
			std::erase(node->widgets, id);
		} else {
			auto childBounds = GetChildBounds(nodeBounds);
			for (std::size_t i = 0; i < 4; ++i) {
				if (node->children[i]) {
					RemoveFromNode(node->children[i].get(), childBounds[i], id, widgetBounds, depth + 1);
				}
			}
		}
	}

	void SplitNode(Node* node, const UIRect& nodeBounds, std::uint32_t depth) {
		node->isLeaf = false;
		auto childBounds = GetChildBounds(nodeBounds);

		for (std::size_t i = 0; i < 4; ++i) {
			node->children[i] = std::make_unique<Node>();
		}

		// Re-insert widgets into children
		for (WidgetID id : node->widgets) {
			if (auto it = entries_.find(id); it != entries_.end()) {
				for (std::size_t i = 0; i < 4; ++i) {
					InsertIntoNode(node->children[i].get(), childBounds[i], id, it->second, depth + 1);
				}
			}
		}
		node->widgets.clear();
	}

	void QueryPointInNode(const Node* node, const UIRect& nodeBounds, const UIVec2& point, std::vector<WidgetID>& results) const {
		if (!nodeBounds.Contains(point)) return;

		if (node->isLeaf) {
			for (WidgetID id : node->widgets) {
				if (auto it = entries_.find(id); it != entries_.end()) {
					if (it->second.Contains(point)) {
						// Avoid duplicates
						if (std::find(results.begin(), results.end(), id) == results.end()) {
							results.push_back(id);
						}
					}
				}
			}
		} else {
			auto childBounds = GetChildBounds(nodeBounds);
			for (std::size_t i = 0; i < 4; ++i) {
				if (node->children[i]) {
					QueryPointInNode(node->children[i].get(), childBounds[i], point, results);
				}
			}
		}
	}

	void QueryRectInNode(const Node* node, const UIRect& nodeBounds, const UIRect& queryRect, std::vector<WidgetID>& results) const {
		if (!nodeBounds.Intersects(queryRect)) return;

		if (node->isLeaf) {
			for (WidgetID id : node->widgets) {
				if (auto it = entries_.find(id); it != entries_.end()) {
					if (it->second.Intersects(queryRect)) {
						if (std::find(results.begin(), results.end(), id) == results.end()) {
							results.push_back(id);
						}
					}
				}
			}
		} else {
			auto childBounds = GetChildBounds(nodeBounds);
			for (std::size_t i = 0; i < 4; ++i) {
				if (node->children[i]) {
					QueryRectInNode(node->children[i].get(), childBounds[i], queryRect, results);
				}
			}
		}
	}

	static std::array<UIRect, 4> GetChildBounds(const UIRect& bounds) {
		double halfW = bounds.size.x * 0.5;
		double halfH = bounds.size.y * 0.5;
		return {{
			{{bounds.position.x, bounds.position.y}, {halfW, halfH}},                      // NW
			{{bounds.position.x + halfW, bounds.position.y}, {halfW, halfH}},              // NE
			{{bounds.position.x, bounds.position.y + halfH}, {halfW, halfH}},              // SW
			{{bounds.position.x + halfW, bounds.position.y + halfH}, {halfW, halfH}}       // SE
		}};
	}

	std::unique_ptr<Node> root_;
	UIRect bounds_;
	std::uint32_t maxDepth_;
	std::uint32_t maxPerNode_;
	std::unordered_map<WidgetID, UIRect> entries_;
};

// ============================================================================
// Simple Linear Spatial Index (for small UIs, < 100 widgets)
// ============================================================================

export class LinearSpatialIndex final : public ISpatialIndex {
public:
	void Insert(WidgetID id, const UIRect& bounds) override {
		entries_[id] = bounds;
	}

	void Remove(WidgetID id) override {
		entries_.erase(id);
	}

	void Update(WidgetID id, const UIRect& newBounds) override {
		entries_[id] = newBounds;
	}

	void Clear() override {
		entries_.clear();
	}

	void QueryPoint(const UIVec2& point, std::vector<WidgetID>& results) const override {
		results.clear();
		for (const auto& [id, bounds] : entries_) {
			if (bounds.Contains(point)) {
				results.push_back(id);
			}
		}
	}

	void QueryRect(const UIRect& rect, std::vector<WidgetID>& results) const override {
		results.clear();
		for (const auto& [id, bounds] : entries_) {
			if (bounds.Intersects(rect)) {
				results.push_back(id);
			}
		}
	}

private:
	std::unordered_map<WidgetID, UIRect> entries_;
};
