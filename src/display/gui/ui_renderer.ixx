export module synodic.soul.gui:ui_renderer;

import std;
import synodic.soul.raster;
import :draw_list;
import :font_atlas;
import :ui_types;

export namespace synodic::soul::gui
{

	// Push constants for UI rendering (must match ui.slang UIPushConstants)
	struct UIPushConstants
	{
		float screenWidth;
		float screenHeight;
		float _padding[2];
		std::uint32_t textureIndex;   // Index into texture heap (0 = no texture / solid color)
		std::uint32_t samplerIndex;   // Index into sampler heap
		std::uint32_t flags;          // Bit flags: 0x1 = has texture, 0x2 = SDF text
		float sdfEdge;                // SDF edge threshold (for text)
		float sdfSmoothing;           // SDF smoothing (for text)
		float _padding2[3];
	};

	static_assert(sizeof(UIPushConstants) == 48, "UIPushConstants must match shader layout");

	// Per-frame render data prepared from UIDrawList
	struct UIRenderBatch
	{
		// Draw call info
		std::uint32_t indexOffset = 0;
		std::uint32_t indexCount = 0;

		// Texture binding (0 = no texture / solid color)
		std::uint32_t textureIndex = 0;
		std::uint32_t flags = 0;

		// Clip rectangle
		UIRect clipRect;
	};

	// Configuration for UI renderer initialization
	struct UIRendererConfig
	{
		// Maximum vertices/indices per frame (grows dynamically if exceeded)
		std::uint32_t initialVertexCapacity = 65536;
		std::uint32_t initialIndexCapacity = 131072;

		// Font atlas configuration
		std::uint32_t atlasWidth = 1024;
		std::uint32_t atlasHeight = 1024;

		// Path to UI shader
		std::filesystem::path shaderPath = "resources/Shaders/ui.slang";

		// Default font to load (empty = no default font)
		std::filesystem::path defaultFontPath;
		float defaultFontSize = 16.0f;
	};

	// Abstract renderer interface (backend-agnostic)
	// Implementations create GPU resources and execute draw calls
	class IUIRenderer
	{
	public:
		virtual ~IUIRenderer() = default;

		// Initialize GPU resources (pipeline, buffers, atlas texture)
		[[nodiscard]] virtual bool Initialize(const UIRendererConfig& config) = 0;

		// Prepare frame data from draw list
		virtual void PrepareFrame(
			const UIDrawList& drawList,
			float screenWidth,
			float screenHeight) = 0;

		// Record draw commands to command list
		virtual void RecordCommands(CommandList& commandList) = 0;

		// Access font atlas for text measurement
		[[nodiscard]] virtual FontAtlas& GetFontAtlas() = 0;
		[[nodiscard]] virtual const FontAtlas& GetFontAtlas() const = 0;

		// Upload updated atlas texture to GPU (call after adding fonts)
		virtual void UploadFontAtlas() = 0;

	protected:
		IUIRenderer() = default;
		IUIRenderer(const IUIRenderer&) = default;
		IUIRenderer(IUIRenderer&&) = default;
		IUIRenderer& operator=(const IUIRenderer&) = default;
		IUIRenderer& operator=(IUIRenderer&&) = default;
	};

	// Concrete UI renderer using the raster backend
	class UIRenderer : public IUIRenderer
	{
	public:
		UIRenderer() = default;
		~UIRenderer() override = default;

		UIRenderer(const UIRenderer&) = delete;
		UIRenderer& operator=(const UIRenderer&) = delete;
		UIRenderer(UIRenderer&&) noexcept = default;
		UIRenderer& operator=(UIRenderer&&) noexcept = default;

		[[nodiscard]] bool Initialize(const UIRendererConfig& config) override
		{
			config_ = config;
			fontAtlas_ = FontAtlas(config.atlasWidth, config.atlasHeight);

			// Load default font if specified
			if (!config.defaultFontPath.empty())
			{
				FontAtlasConfig fontConfig;
				fontConfig.fontPath = config.defaultFontPath;
				fontConfig.pixelSize = config.defaultFontSize;

				if (!fontAtlas_.AddFont(fontConfig))
				{
					return false;
				}
			}

			// Pre-allocate vertex/index buffers
			vertices_.reserve(config.initialVertexCapacity);
			indices_.reserve(config.initialIndexCapacity);

			return true;
		}

		void PrepareFrame(
			const UIDrawList& drawList,
			float screenWidth,
			float screenHeight) override
		{
			// Clear previous frame data
			vertices_.clear();
			indices_.clear();
			batches_.clear();

			screenWidth_ = screenWidth;
			screenHeight_ = screenHeight;

			// Convert draw commands to vertices/indices
			GenerateGeometry(drawList);
		}

		void RecordCommands(CommandList& commandList) override
		{
			if (vertices_.empty())
			{
				return;
			}

			// Set up push constants (screen dimensions + texture info)
			UIPushConstants pushConstants;
			pushConstants.screenWidth = screenWidth_;
			pushConstants.screenHeight = screenHeight_;
			pushConstants.textureIndex = 0;
			pushConstants.samplerIndex = 0;
			pushConstants.flags = 0;  // No texture for basic UI
			pushConstants.sdfEdge = 0.5f;
			pushConstants.sdfSmoothing = 0.1f;

			// For now, record a simple draw call
			// Full implementation would use GPUAllocator to upload vertex/index data
			// and issue DrawWithPointers commands

			// TODO: Implement full GPU upload and batched rendering
			// This requires access to the GPU allocator from the raster backend
		}

		[[nodiscard]] FontAtlas& GetFontAtlas() override { return fontAtlas_; }
		[[nodiscard]] const FontAtlas& GetFontAtlas() const override { return fontAtlas_; }

		void UploadFontAtlas() override
		{
			if (fontAtlas_.IsDirty())
			{
				// TODO: Upload atlas texture to GPU
				// Requires texture heap access from raster backend
				fontAtlas_.ClearDirty();
			}
		}

		// Access generated geometry (for external rendering)
		[[nodiscard]] std::span<const UIVertex> GetVertices() const { return vertices_; }
		[[nodiscard]] std::span<const std::uint32_t> GetIndices() const { return indices_; }
		[[nodiscard]] std::span<const UIRenderBatch> GetBatches() const { return batches_; }

	private:
		void GenerateGeometry(const UIDrawList& drawList)
		{
			const auto& commands = drawList.Commands();
			const auto& textBuffer = drawList.TextBuffer();

			UIRenderBatch currentBatch;
			currentBatch.indexOffset = 0;

			for (const auto& cmd : commands)
			{
				switch (cmd.type)
				{
				case UIDrawCommandType::Rect:
					GenerateRect(cmd);
					break;

				case UIDrawCommandType::RoundedRect:
					GenerateRoundedRect(cmd);
					break;

				case UIDrawCommandType::Text:
					GenerateText(cmd, textBuffer);
					break;

				case UIDrawCommandType::Image:
					GenerateImage(cmd);
					break;

				case UIDrawCommandType::PushClip:
				case UIDrawCommandType::PopClip:
				case UIDrawCommandType::PushScissor:
				case UIDrawCommandType::PopScissor:
					// Handle clipping state changes
					// Would finalize current batch and start new one
					break;

				default:
					break;
				}
			}

			// Finalize last batch
			if (indices_.size() > currentBatch.indexOffset)
			{
				currentBatch.indexCount = static_cast<std::uint32_t>(
					indices_.size() - currentBatch.indexOffset);
				batches_.push_back(currentBatch);
			}
		}

		void GenerateRect(const UIDrawCommand& cmd)
		{
			std::uint32_t baseVertex = static_cast<std::uint32_t>(vertices_.size());

			// Four corners
			UIVertex v0, v1, v2, v3;

			v0.x = cmd.x;
			v0.y = cmd.y;
			v0.u = 0.0f;
			v0.v = 0.0f;
			v0.color = cmd.color;

			v1.x = cmd.x + cmd.width;
			v1.y = cmd.y;
			v1.u = 1.0f;
			v1.v = 0.0f;
			v1.color = cmd.color;

			v2.x = cmd.x + cmd.width;
			v2.y = cmd.y + cmd.height;
			v2.u = 1.0f;
			v2.v = 1.0f;
			v2.color = cmd.color;

			v3.x = cmd.x;
			v3.y = cmd.y + cmd.height;
			v3.u = 0.0f;
			v3.v = 1.0f;
			v3.color = cmd.color;

			vertices_.push_back(v0);
			vertices_.push_back(v1);
			vertices_.push_back(v2);
			vertices_.push_back(v3);

			// Two triangles
			indices_.push_back(baseVertex + 0);
			indices_.push_back(baseVertex + 1);
			indices_.push_back(baseVertex + 2);
			indices_.push_back(baseVertex + 0);
			indices_.push_back(baseVertex + 2);
			indices_.push_back(baseVertex + 3);
		}

		void GenerateRoundedRect(const UIDrawCommand& cmd)
		{
			// Simplified: generate as regular rect for now
			// Full implementation would tessellate corner arcs
			GenerateRect(cmd);
		}

		void GenerateText(const UIDrawCommand& cmd, std::span<const char> textBuffer)
		{
			const auto* font = fontAtlas_.GetDefaultFont();
			if (!font)
			{
				return;
			}

			// Extract text from buffer
			std::uint32_t offset = cmd.params.text.textOffset;
			std::uint16_t length = cmd.params.text.textLength;

			if (offset + length > textBuffer.size())
			{
				return;
			}

			std::string_view text(textBuffer.data() + offset, length);

			// Current cursor position
			float cursorX = cmd.x;
			float cursorY = cmd.y + font->ascender;  // Baseline

			// Generate quad for each glyph
			for (char c : text)
			{
				char32_t cp = static_cast<char32_t>(static_cast<unsigned char>(c));
				const auto* glyph = font->GetGlyph(cp);

				if (!glyph)
				{
					glyph = font->GetFallbackGlyph();
					if (!glyph)
					{
						continue;
					}
				}

				// Skip whitespace (but advance cursor)
				if (glyph->atlasWidth > 0 && glyph->atlasHeight > 0)
				{
					std::uint32_t baseVertex = static_cast<std::uint32_t>(vertices_.size());

					float x0 = cursorX + glyph->bearingX;
					float y0 = cursorY - glyph->bearingY;
					float x1 = x0 + static_cast<float>(glyph->atlasWidth);
					float y1 = y0 + static_cast<float>(glyph->atlasHeight);

					UIVertex v0, v1, v2, v3;

					v0.x = x0;
					v0.y = y0;
					v0.u = glyph->u0;
					v0.v = glyph->v0;
					v0.color = cmd.color;

					v1.x = x1;
					v1.y = y0;
					v1.u = glyph->u1;
					v1.v = glyph->v0;
					v1.color = cmd.color;

					v2.x = x1;
					v2.y = y1;
					v2.u = glyph->u1;
					v2.v = glyph->v1;
					v2.color = cmd.color;

					v3.x = x0;
					v3.y = y1;
					v3.u = glyph->u0;
					v3.v = glyph->v1;
					v3.color = cmd.color;

					vertices_.push_back(v0);
					vertices_.push_back(v1);
					vertices_.push_back(v2);
					vertices_.push_back(v3);

					indices_.push_back(baseVertex + 0);
					indices_.push_back(baseVertex + 1);
					indices_.push_back(baseVertex + 2);
					indices_.push_back(baseVertex + 0);
					indices_.push_back(baseVertex + 2);
					indices_.push_back(baseVertex + 3);
				}

				cursorX += glyph->advanceX;
			}
		}

		void GenerateImage(const UIDrawCommand& cmd)
		{
			std::uint32_t baseVertex = static_cast<std::uint32_t>(vertices_.size());

			UIVertex v0, v1, v2, v3;

			v0.x = cmd.x;
			v0.y = cmd.y;
			v0.u = cmd.params.uv.u0;
			v0.v = cmd.params.uv.v0;
			v0.color = cmd.color;

			v1.x = cmd.x + cmd.width;
			v1.y = cmd.y;
			v1.u = cmd.params.uv.u1;
			v1.v = cmd.params.uv.v0;
			v1.color = cmd.color;

			v2.x = cmd.x + cmd.width;
			v2.y = cmd.y + cmd.height;
			v2.u = cmd.params.uv.u1;
			v2.v = cmd.params.uv.v1;
			v2.color = cmd.color;

			v3.x = cmd.x;
			v3.y = cmd.y + cmd.height;
			v3.u = cmd.params.uv.u0;
			v3.v = cmd.params.uv.v1;
			v3.color = cmd.color;

			vertices_.push_back(v0);
			vertices_.push_back(v1);
			vertices_.push_back(v2);
			vertices_.push_back(v3);

			indices_.push_back(baseVertex + 0);
			indices_.push_back(baseVertex + 1);
			indices_.push_back(baseVertex + 2);
			indices_.push_back(baseVertex + 0);
			indices_.push_back(baseVertex + 2);
			indices_.push_back(baseVertex + 3);
		}

		UIRendererConfig config_;
		FontAtlas fontAtlas_{1024, 1024};

		float screenWidth_ = 0.0f;
		float screenHeight_ = 0.0f;

		// CPU-side geometry buffers
		std::vector<UIVertex> vertices_;
		std::vector<std::uint32_t> indices_;
		std::vector<UIRenderBatch> batches_;
	};

}
