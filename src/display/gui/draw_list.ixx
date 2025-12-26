export module synodic.soul.gui:draw_list;

import std;
import :ui_types;

// ============================================================================
// UI Draw Command Types
// ============================================================================

export enum class UIDrawCommandType : std::uint8_t {
	// Primitives
	Rect,
	RoundedRect,
	Circle,
	Line,
	
	// Text
	Text,
	
	// Textures
	Image,
	Icon,
	
	// Clipping (for scrollable regions, dropdowns)
	PushClip,
	PopClip,
	
	// Scissor (for dropdown overflow)
	PushScissor,
	PopScissor
};

// ============================================================================
// Draw Command - GPU-friendly layout (48 bytes, cache-line aligned)
// ============================================================================

export struct UIDrawCommand {
	UIDrawCommandType type;
	std::uint8_t flags = 0;          // Bitflags for options
	std::uint16_t textureIndex = 0;  // For image/icon commands
	
	// Bounds (position + size packed as floats for GPU)
	float x, y, width, height;
	
	// Color (premultiplied alpha for efficient blending)
	std::uint32_t color = 0xFFFFFFFF;  // RGBA packed
	
	// Extra params (context-dependent)
	union {
		struct { float cornerRadius; float borderWidth; } rect;
		struct { std::uint32_t textOffset; std::uint16_t textLength; std::uint16_t fontSize; } text;
		struct { float u0, v0, u1, v1; } uv;  // Texture coordinates
	} params = {};
	
	// Padding for 48-byte alignment
	std::uint32_t _pad[2] = {0, 0};
};

static_assert(sizeof(UIDrawCommand) == 48, "UIDrawCommand should be 48 bytes for cache efficiency");

// ============================================================================
// Vertex for UI rendering (20 bytes - position, UV, color)
// ============================================================================

export struct UIVertex {
	float x, y;           // Position (screen space)
	float u, v;           // Texture coordinates
	std::uint32_t color;  // RGBA packed
};

static_assert(sizeof(UIVertex) == 20, "UIVertex should be 20 bytes");

// ============================================================================
// UI Draw List - Batched command buffer for render graph integration
// ============================================================================

export class UIDrawList {
public:
	UIDrawList() {
		// Pre-allocate for typical UI complexity
		commands_.reserve(256);
		textBuffer_.reserve(4096);
		clipStack_.reserve(8);
	}
	
	// =========================================================================
	// Frame Management
	// =========================================================================
	
	void Clear() {
		commands_.clear();
		textBuffer_.clear();
		clipStack_.clear();
		vertexCount_ = 0;
		indexCount_ = 0;
	}
	
	[[nodiscard]] bool IsEmpty() const { return commands_.empty(); }
	
	// =========================================================================
	// Primitive Drawing
	// =========================================================================
	
	void AddRect(const UIRect& bounds, std::uint32_t color) {
		UIDrawCommand cmd;
		cmd.type = UIDrawCommandType::Rect;
		cmd.x = static_cast<float>(bounds.position.x);
		cmd.y = static_cast<float>(bounds.position.y);
		cmd.width = static_cast<float>(bounds.size.x);
		cmd.height = static_cast<float>(bounds.size.y);
		cmd.color = color;
		commands_.push_back(cmd);
		
		vertexCount_ += 4;
		indexCount_ += 6;
	}
	
	void AddRoundedRect(const UIRect& bounds, std::uint32_t color, float cornerRadius) {
		UIDrawCommand cmd;
		cmd.type = UIDrawCommandType::RoundedRect;
		cmd.x = static_cast<float>(bounds.position.x);
		cmd.y = static_cast<float>(bounds.position.y);
		cmd.width = static_cast<float>(bounds.size.x);
		cmd.height = static_cast<float>(bounds.size.y);
		cmd.color = color;
		cmd.params.rect.cornerRadius = cornerRadius;
		commands_.push_back(cmd);
		
		// Rounded rect uses more vertices for smooth corners
		vertexCount_ += 16;  // Approximate
		indexCount_ += 42;
	}
	
	void AddBorderedRect(const UIRect& bounds, std::uint32_t fillColor, 
	                     std::uint32_t borderColor, float borderWidth, float cornerRadius = 0.0f) {
		// Border (slightly larger)
		UIRect borderBounds = bounds;
		AddRoundedRect(borderBounds, borderColor, cornerRadius);
		
		// Fill (inset by border width)
		UIRect fillBounds = {
			{bounds.position.x + borderWidth, bounds.position.y + borderWidth},
			{bounds.size.x - borderWidth * 2, bounds.size.y - borderWidth * 2}
		};
		AddRoundedRect(fillBounds, fillColor, std::max(0.0f, cornerRadius - borderWidth));
	}
	
	// =========================================================================
	// Text Drawing
	// =========================================================================
	
	void AddText(const UIRect& bounds, std::string_view text, std::uint32_t color, 
	             std::uint16_t fontSize = 14) {
		if (text.empty()) return;
		
		UIDrawCommand cmd;
		cmd.type = UIDrawCommandType::Text;
		cmd.x = static_cast<float>(bounds.position.x);
		cmd.y = static_cast<float>(bounds.position.y);
		cmd.width = static_cast<float>(bounds.size.x);
		cmd.height = static_cast<float>(bounds.size.y);
		cmd.color = color;
		cmd.params.text.textOffset = static_cast<std::uint32_t>(textBuffer_.size());
		cmd.params.text.textLength = static_cast<std::uint16_t>(std::min(text.size(), size_t{65535}));
		cmd.params.text.fontSize = fontSize;
		
		textBuffer_.append(text);
		commands_.push_back(cmd);
		
		// Estimate vertex count (6 per glyph)
		vertexCount_ += static_cast<std::uint32_t>(text.size() * 4);
		indexCount_ += static_cast<std::uint32_t>(text.size() * 6);
	}
	
	// =========================================================================
	// Clipping (hierarchical, for nested scroll views)
	// =========================================================================
	
	void PushClipRect(const UIRect& bounds) {
		UIDrawCommand cmd;
		cmd.type = UIDrawCommandType::PushClip;
		cmd.x = static_cast<float>(bounds.position.x);
		cmd.y = static_cast<float>(bounds.position.y);
		cmd.width = static_cast<float>(bounds.size.x);
		cmd.height = static_cast<float>(bounds.size.y);
		commands_.push_back(cmd);
		
		clipStack_.push_back(bounds);
	}
	
	void PopClipRect() {
		if (!clipStack_.empty()) {
			UIDrawCommand cmd;
			cmd.type = UIDrawCommandType::PopClip;
			commands_.push_back(cmd);
			clipStack_.pop_back();
		}
	}
	
	[[nodiscard]] std::optional<UIRect> CurrentClipRect() const {
		return clipStack_.empty() ? std::nullopt : std::optional{clipStack_.back()};
	}
	
	// =========================================================================
	// Scissor (GPU-level clipping, for dropdown menus extending beyond parent)
	// =========================================================================
	
	void PushScissor(const UIRect& bounds) {
		UIDrawCommand cmd;
		cmd.type = UIDrawCommandType::PushScissor;
		cmd.x = static_cast<float>(bounds.position.x);
		cmd.y = static_cast<float>(bounds.position.y);
		cmd.width = static_cast<float>(bounds.size.x);
		cmd.height = static_cast<float>(bounds.size.y);
		commands_.push_back(cmd);
	}
	
	void PopScissor() {
		UIDrawCommand cmd;
		cmd.type = UIDrawCommandType::PopScissor;
		commands_.push_back(cmd);
	}
	
	// =========================================================================
	// Batch Access (for render graph integration)
	// =========================================================================
	
	[[nodiscard]] std::span<const UIDrawCommand> Commands() const { 
		return commands_; 
	}
	
	[[nodiscard]] std::string_view TextBuffer() const { 
		return textBuffer_; 
	}
	
	[[nodiscard]] std::uint32_t EstimatedVertexCount() const { return vertexCount_; }
	[[nodiscard]] std::uint32_t EstimatedIndexCount() const { return indexCount_; }
	
	// Size in bytes for GPU buffer allocation
	[[nodiscard]] std::size_t CommandBufferSize() const {
		return commands_.size() * sizeof(UIDrawCommand);
	}
	
private:
	std::vector<UIDrawCommand> commands_;
	std::string textBuffer_;
	std::vector<UIRect> clipStack_;
	
	// Pre-computed counts for buffer allocation
	std::uint32_t vertexCount_ = 0;
	std::uint32_t indexCount_ = 0;
};

// ============================================================================
// UI Render Data - Complete package for render pass
// ============================================================================

export struct UIRenderData {
	const UIDrawList* drawList = nullptr;
	
	// Viewport info
	float viewportWidth = 0;
	float viewportHeight = 0;
	
	// Font atlas texture handle (for text rendering)
	std::uint32_t fontAtlasTexture = 0;
	
	// Transform (for resolution-independent UI)
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float offsetX = 0.0f;
	float offsetY = 0.0f;
};
