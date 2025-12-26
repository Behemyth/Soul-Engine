export module synodic.soul.gui:font_atlas;

import std;
import synodic.soul.transput;

export namespace synodic::soul::gui
{

	// Single glyph region in the atlas
	struct AtlasGlyph
	{
		char32_t codepoint = 0;

		// Position in atlas texture (pixels)
		std::uint16_t atlasX = 0;
		std::uint16_t atlasY = 0;
		std::uint16_t atlasWidth = 0;
		std::uint16_t atlasHeight = 0;

		// Rendering metrics (in pixels at rasterized size)
		float bearingX = 0.0f;
		float bearingY = 0.0f;
		float advanceX = 0.0f;

		// Normalized UV coordinates (computed from atlas position)
		float u0 = 0.0f;
		float v0 = 0.0f;
		float u1 = 0.0f;
		float v1 = 0.0f;
	};

	// Font configuration for atlas generation
	struct FontAtlasConfig
	{
		std::filesystem::path fontPath;
		float pixelSize = 16.0f;
		font::FontWeight weight = font::FontWeight::Regular;
		font::FontStyle style   = font::FontStyle::Normal;

		// Characters to pre-rasterize
		std::u32string characters = U" !\"#$%&'()*+,-./0123456789:;<=>?@"
		                            U"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
		                            U"abcdefghijklmnopqrstuvwxyz{|}~";
	};

	// Font entry in atlas (one per font/size combination)
	struct AtlasFont
	{
		std::string name;
		float pixelSize = 0.0f;
		font::FontWeight weight = font::FontWeight::Regular;
		font::FontStyle style   = font::FontStyle::Normal;

		// Font metrics at this size
		float ascender   = 0.0f;
		float descender  = 0.0f;
		float lineHeight = 0.0f;

		// Glyph lookup (codepoint -> glyph info)
		std::unordered_map<char32_t, AtlasGlyph> glyphs;

		// Get glyph for codepoint (returns nullptr if not found)
		[[nodiscard]] const AtlasGlyph* GetGlyph(char32_t cp) const
		{
			auto it = glyphs.find(cp);
			return it != glyphs.end() ? &it->second : nullptr;
		}

		// Get fallback glyph (typically '?')
		[[nodiscard]] const AtlasGlyph* GetFallbackGlyph() const
		{
			return GetGlyph(U'?');
		}
	};

	// Rectangle bin packer for atlas layout (simple shelf algorithm)
	class RectPacker
	{
	public:
		explicit RectPacker(std::uint32_t width, std::uint32_t height)
			: width_(width), height_(height), currentX_(0), currentY_(0), rowHeight_(0)
		{
		}

		// Try to pack a rectangle, returns position or nullopt if full
		[[nodiscard]] std::optional<std::pair<std::uint32_t, std::uint32_t>>
		Pack(std::uint32_t w, std::uint32_t h)
		{
			// Add padding between glyphs
			w += 1;
			h += 1;

			// Check if we need to start a new row
			if (currentX_ + w > width_)
			{
				currentX_ = 0;
				currentY_ += rowHeight_;
				rowHeight_ = 0;
			}

			// Check if atlas is full
			if (currentY_ + h > height_)
			{
				return std::nullopt;
			}

			std::uint32_t x = currentX_;
			std::uint32_t y = currentY_;

			currentX_ += w;
			rowHeight_ = std::max(rowHeight_, h);

			return std::pair{x, y};
		}

		void Reset()
		{
			currentX_ = 0;
			currentY_ = 0;
			rowHeight_ = 0;
		}

	private:
		std::uint32_t width_;
		std::uint32_t height_;
		std::uint32_t currentX_;
		std::uint32_t currentY_;
		std::uint32_t rowHeight_;
	};

	// Font atlas: packs multiple font faces/sizes into a single texture
	class FontAtlas
	{
	public:
		explicit FontAtlas(std::uint32_t width = 1024, std::uint32_t height = 1024)
			: width_(width)
			, height_(height)
			, packer_(width, height)
		{
			// Initialize grayscale texture data
			textureData_.resize(static_cast<std::size_t>(width) * height, 0);
		}

		// Add a font to the atlas with specified characters
		bool AddFont(const FontAtlasConfig& config)
		{
			auto faceResult = font::FontFace::LoadFromFile(config.fontPath);
			if (!faceResult)
			{
				return false;
			}

			auto& face = *faceResult;
			[[maybe_unused]] auto _ = face.SetPixelSize(config.pixelSize);

			AtlasFont atlasFont;
			atlasFont.name      = face.GetInfo().familyName;
			atlasFont.pixelSize = config.pixelSize;
			atlasFont.weight    = config.weight;
			atlasFont.style     = config.style;

			auto metrics        = face.GetMetrics();
			atlasFont.ascender   = metrics.ascender;
			atlasFont.descender  = metrics.descender;
			atlasFont.lineHeight = metrics.lineHeight;

			// Rasterize each character
			for (char32_t cp : config.characters)
			{
				auto glyphResult = face.RasterizeGlyph(cp);
				if (!glyphResult)
				{
					continue;
				}

				const auto& glyph = *glyphResult;

				// Skip empty glyphs (spaces, etc.)
				if (glyph.metrics.width == 0 || glyph.metrics.height == 0)
				{
					// Still add to map with zero-size for advance info
					AtlasGlyph ag;
					ag.codepoint = cp;
					ag.advanceX  = static_cast<float>(glyph.metrics.advanceX);
					atlasFont.glyphs[cp] = ag;
					continue;
				}

				// Pack into atlas
				auto pos = packer_.Pack(glyph.metrics.width, glyph.metrics.height);
				if (!pos)
				{
					// Atlas full - could resize or fail
					return false;
				}

				auto [x, y] = *pos;

				// Copy glyph bitmap to atlas texture
				for (std::uint32_t row = 0; row < glyph.metrics.height; ++row)
				{
					std::size_t srcOffset = row * glyph.pitch;
					std::size_t dstOffset = (y + row) * width_ + x;

					std::memcpy(
						textureData_.data() + dstOffset,
						glyph.data.data() + srcOffset,
						glyph.metrics.width);
				}

				// Create atlas glyph entry
				AtlasGlyph ag;
				ag.codepoint   = cp;
				ag.atlasX      = static_cast<std::uint16_t>(x);
				ag.atlasY      = static_cast<std::uint16_t>(y);
				ag.atlasWidth  = static_cast<std::uint16_t>(glyph.metrics.width);
				ag.atlasHeight = static_cast<std::uint16_t>(glyph.metrics.height);
				ag.bearingX    = static_cast<float>(glyph.metrics.bearingX);
				ag.bearingY    = static_cast<float>(glyph.metrics.bearingY);
				ag.advanceX    = static_cast<float>(glyph.metrics.advanceX);

				// Compute UVs
				float invW = 1.0f / static_cast<float>(width_);
				float invH = 1.0f / static_cast<float>(height_);
				ag.u0 = static_cast<float>(x) * invW;
				ag.v0 = static_cast<float>(y) * invH;
				ag.u1 = static_cast<float>(x + glyph.metrics.width) * invW;
				ag.v1 = static_cast<float>(y + glyph.metrics.height) * invH;

				atlasFont.glyphs[cp] = ag;
			}

			fonts_.push_back(std::move(atlasFont));
			dirty_ = true;
			return true;
		}

		// Find font in atlas by name and size
		[[nodiscard]] const AtlasFont* FindFont(
			std::string_view name,
			float pixelSize,
			font::FontWeight weight = font::FontWeight::Regular,
			font::FontStyle style   = font::FontStyle::Normal) const
		{
			for (const auto& font : fonts_)
			{
				if (font.name == name &&
					std::abs(font.pixelSize - pixelSize) < 0.5f &&
					font.weight == weight &&
					font.style == style)
				{
					return &font;
				}
			}
			return nullptr;
		}

		// Get default font (first added)
		[[nodiscard]] const AtlasFont* GetDefaultFont() const
		{
			return fonts_.empty() ? nullptr : &fonts_.front();
		}

		// Accessors
		[[nodiscard]] std::uint32_t Width() const { return width_; }
		[[nodiscard]] std::uint32_t Height() const { return height_; }
		[[nodiscard]] std::span<const std::uint8_t> TextureData() const { return textureData_; }
		[[nodiscard]] bool IsDirty() const { return dirty_; }
		void ClearDirty() { dirty_ = false; }

		// Get all fonts in atlas
		[[nodiscard]] std::span<const AtlasFont> GetFonts() const { return fonts_; }

		// Measure text dimensions
		[[nodiscard]] std::pair<float, float> MeasureText(
			std::u32string_view text,
			const AtlasFont* font) const
		{
			if (!font)
			{
				return {0.0f, 0.0f};
			}

			float width = 0.0f;
			float maxHeight = font->lineHeight;

			for (char32_t cp : text)
			{
				if (const auto* glyph = font->GetGlyph(cp))
				{
					width += glyph->advanceX;
				}
				else if (const auto* fallback = font->GetFallbackGlyph())
				{
					width += fallback->advanceX;
				}
			}

			return {width, maxHeight};
		}

		// UTF-8 overload
		[[nodiscard]] std::pair<float, float> MeasureText(
			std::string_view text,
			const AtlasFont* font) const
		{
			// Convert UTF-8 to UTF-32
			std::u32string u32text;
			u32text.reserve(text.size());

			std::size_t i = 0;
			while (i < text.size())
			{
				char32_t cp = 0;
				unsigned char c = static_cast<unsigned char>(text[i]);

				if ((c & 0x80) == 0)
				{
					cp = c;
					i += 1;
				}
				else if ((c & 0xE0) == 0xC0)
				{
					cp = (c & 0x1F) << 6;
					if (i + 1 < text.size())
						cp |= (static_cast<unsigned char>(text[i + 1]) & 0x3F);
					i += 2;
				}
				else if ((c & 0xF0) == 0xE0)
				{
					cp = (c & 0x0F) << 12;
					if (i + 1 < text.size())
						cp |= (static_cast<unsigned char>(text[i + 1]) & 0x3F) << 6;
					if (i + 2 < text.size())
						cp |= (static_cast<unsigned char>(text[i + 2]) & 0x3F);
					i += 3;
				}
				else if ((c & 0xF8) == 0xF0)
				{
					cp = (c & 0x07) << 18;
					if (i + 1 < text.size())
						cp |= (static_cast<unsigned char>(text[i + 1]) & 0x3F) << 12;
					if (i + 2 < text.size())
						cp |= (static_cast<unsigned char>(text[i + 2]) & 0x3F) << 6;
					if (i + 3 < text.size())
						cp |= (static_cast<unsigned char>(text[i + 3]) & 0x3F);
					i += 4;
				}
				else
				{
					++i;  // Skip invalid byte
					continue;
				}

				u32text.push_back(cp);
			}

			return MeasureText(u32text, font);
		}

	private:
		std::uint32_t width_;
		std::uint32_t height_;
		RectPacker packer_;

		// CPU-side texture data (grayscale, 8-bit)
		std::vector<std::uint8_t> textureData_;

		// All fonts in this atlas
		std::vector<AtlasFont> fonts_;

		// Dirty flag for GPU upload
		bool dirty_ = false;
	};

}
