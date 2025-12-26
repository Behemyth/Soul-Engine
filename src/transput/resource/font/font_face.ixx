module;

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H

export module synodic.soul.transput:font_face;

import std;
import :font_types;

export namespace synodic::soul::font
{

	// RAII wrapper for FreeType library instance
	class FreeTypeLibrary
	{
	public:
		FreeTypeLibrary()
		{
			if (FT_Init_FreeType(&library_) != 0)
			{
				library_ = nullptr;
			}
		}

		~FreeTypeLibrary()
		{
			if (library_)
			{
				FT_Done_FreeType(library_);
			}
		}

		FreeTypeLibrary(const FreeTypeLibrary&)            = delete;
		FreeTypeLibrary& operator=(const FreeTypeLibrary&) = delete;

		FreeTypeLibrary(FreeTypeLibrary&& other) noexcept : library_(other.library_)
		{
			other.library_ = nullptr;
		}

		FreeTypeLibrary& operator=(FreeTypeLibrary&& other) noexcept
		{
			if (this != &other)
			{
				if (library_)
				{
					FT_Done_FreeType(library_);
				}
				library_       = other.library_;
				other.library_ = nullptr;
			}
			return *this;
		}

		[[nodiscard]] FT_Library Get() const { return library_; }
		[[nodiscard]] bool IsValid() const { return library_ != nullptr; }

		// Singleton access for shared library instance
		static FreeTypeLibrary& Instance()
		{
			static FreeTypeLibrary instance;
			return instance;
		}

	private:
		FT_Library library_ = nullptr;
	};

	// RAII wrapper for a loaded font face
	class FontFace
	{
	public:
		FontFace() = default;

		~FontFace()
		{
			if (face_)
			{
				FT_Done_Face(face_);
			}
		}

		FontFace(const FontFace&)            = delete;
		FontFace& operator=(const FontFace&) = delete;

		FontFace(FontFace&& other) noexcept
			: face_(other.face_)
			, fontData_(std::move(other.fontData_))
			, info_(std::move(other.info_))
			, currentPixelSize_(other.currentPixelSize_)
		{
			other.face_ = nullptr;
		}

		FontFace& operator=(FontFace&& other) noexcept
		{
			if (this != &other)
			{
				if (face_)
				{
					FT_Done_Face(face_);
				}
				face_             = other.face_;
				fontData_         = std::move(other.fontData_);
				info_             = std::move(other.info_);
				currentPixelSize_ = other.currentPixelSize_;
				other.face_       = nullptr;
			}
			return *this;
		}

		// Load font from file path
		[[nodiscard]] static FontResult<FontFace> LoadFromFile(
			const std::filesystem::path& path,
			std::int32_t faceIndex = 0)
		{
			auto& library = FreeTypeLibrary::Instance();
			if (!library.IsValid())
			{
				return std::unexpected(FontError::FreeTypeError);
			}

			// Read entire file into memory (FreeType needs data to persist)
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (!file)
			{
				return std::unexpected(FontError::FileNotFound);
			}

			auto size = file.tellg();
			file.seekg(0);

			FontFace result;
			result.fontData_.resize(static_cast<std::size_t>(size));
			file.read(reinterpret_cast<char*>(result.fontData_.data()), size);

			FT_Error error = FT_New_Memory_Face(
				library.Get(),
				result.fontData_.data(),
				static_cast<FT_Long>(result.fontData_.size()),
				faceIndex,
				&result.face_);

			if (error != 0)
			{
				return std::unexpected(FontError::InvalidFormat);
			}

			result.ExtractFontInfo();
			return result;
		}

		// Load font from memory buffer
		[[nodiscard]] static FontResult<FontFace> LoadFromMemory(
			std::span<const std::uint8_t> data,
			std::int32_t faceIndex = 0)
		{
			auto& library = FreeTypeLibrary::Instance();
			if (!library.IsValid())
			{
				return std::unexpected(FontError::FreeTypeError);
			}

			FontFace result;
			result.fontData_.assign(data.begin(), data.end());

			FT_Error error = FT_New_Memory_Face(
				library.Get(),
				result.fontData_.data(),
				static_cast<FT_Long>(result.fontData_.size()),
				faceIndex,
				&result.face_);

			if (error != 0)
			{
				return std::unexpected(FontError::InvalidFormat);
			}

			result.ExtractFontInfo();
			return result;
		}

		// Set pixel size for rendering
		[[nodiscard]] FontResult<void> SetPixelSize(float pixelSize, std::uint32_t dpiX = 96, std::uint32_t dpiY = 96)
		{
			if (!face_)
			{
				return std::unexpected(FontError::FreeTypeError);
			}

			// Convert to 26.6 fixed point
			FT_Error error = FT_Set_Char_Size(
				face_,
				0,                                           // width in 1/64 points (0 = same as height)
				static_cast<FT_F26Dot6>(pixelSize * 64.0f),  // height in 1/64 points
				dpiX,
				dpiY);

			if (error != 0)
			{
				return std::unexpected(FontError::InvalidSize);
			}

			currentPixelSize_ = pixelSize;
			return {};
		}

		// Get scaled font metrics for current pixel size
		[[nodiscard]] FontMetrics GetMetrics() const
		{
			FontMetrics metrics{};

			if (!face_)
			{
				return metrics;
			}

			// FreeType metrics are in 26.6 fixed point after SetPixelSize
			float scale = 1.0f / 64.0f;

			metrics.ascender   = static_cast<float>(face_->size->metrics.ascender) * scale;
			metrics.descender  = static_cast<float>(face_->size->metrics.descender) * scale;
			metrics.lineHeight = static_cast<float>(face_->size->metrics.height) * scale;
			metrics.lineGap    = metrics.lineHeight - (metrics.ascender - metrics.descender);

			metrics.unitsPerEm       = static_cast<float>(face_->units_per_EM);
			metrics.maxAdvanceWidth  = static_cast<float>(face_->size->metrics.max_advance) * scale;
			metrics.maxAdvanceHeight = metrics.lineHeight;

			// Underline/strikeout (from OS/2 table, scaled)
			float designScale = currentPixelSize_ / metrics.unitsPerEm;
			metrics.underlinePosition  = static_cast<float>(face_->underline_position) * designScale;
			metrics.underlineThickness = static_cast<float>(face_->underline_thickness) * designScale;

			// Approximate strikeout position (typically around x-height / 2)
			metrics.strikeoutPosition  = metrics.ascender * 0.3f;
			metrics.strikeoutThickness = metrics.underlineThickness;

			return metrics;
		}

		// Rasterize a single glyph
		[[nodiscard]] FontResult<GlyphBitmap> RasterizeGlyph(char32_t codepoint) const
		{
			if (!face_)
			{
				return std::unexpected(FontError::FreeTypeError);
			}

			FT_UInt glyphIndex = FT_Get_Char_Index(face_, codepoint);
			if (glyphIndex == 0 && codepoint != 0)
			{
				return std::unexpected(FontError::GlyphNotFound);
			}

			FT_Error error = FT_Load_Glyph(face_, glyphIndex, FT_LOAD_DEFAULT);
			if (error != 0)
			{
				return std::unexpected(FontError::FreeTypeError);
			}

			error = FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_NORMAL);
			if (error != 0)
			{
				return std::unexpected(FontError::FreeTypeError);
			}

			const FT_GlyphSlot& glyph = face_->glyph;
			const FT_Bitmap& bitmap   = glyph->bitmap;

			GlyphBitmap result;
			result.metrics.glyphIndex = glyphIndex;
			result.metrics.codepoint  = codepoint;
			result.metrics.width      = bitmap.width;
			result.metrics.height     = bitmap.rows;
			result.metrics.bearingX   = glyph->bitmap_left;
			result.metrics.bearingY   = glyph->bitmap_top;
			result.metrics.advanceX   = static_cast<std::int32_t>(glyph->advance.x >> 6);
			result.metrics.advanceY   = static_cast<std::int32_t>(glyph->advance.y >> 6);

			result.pitch = bitmap.pitch;

			// Copy bitmap data
			if (bitmap.buffer && bitmap.rows > 0 && bitmap.width > 0)
			{
				std::size_t dataSize = static_cast<std::size_t>(bitmap.rows) * bitmap.pitch;
				result.data.resize(dataSize);
				std::memcpy(result.data.data(), bitmap.buffer, dataSize);
			}

			return result;
		}

		// Rasterize multiple glyphs efficiently
		[[nodiscard]] std::vector<GlyphBitmap> RasterizeGlyphs(std::u32string_view codepoints) const
		{
			std::vector<GlyphBitmap> results;
			results.reserve(codepoints.size());

			for (char32_t cp : codepoints)
			{
				if (auto glyph = RasterizeGlyph(cp))
				{
					results.push_back(std::move(*glyph));
				}
			}

			return results;
		}

		// Get kerning between two glyphs (in pixels)
		[[nodiscard]] float GetKerning(char32_t left, char32_t right) const
		{
			if (!face_ || !FT_HAS_KERNING(face_))
			{
				return 0.0f;
			}

			FT_UInt leftIndex  = FT_Get_Char_Index(face_, left);
			FT_UInt rightIndex = FT_Get_Char_Index(face_, right);

			FT_Vector kerning;
			FT_Error error = FT_Get_Kerning(face_, leftIndex, rightIndex, FT_KERNING_DEFAULT, &kerning);

			if (error != 0)
			{
				return 0.0f;
			}

			return static_cast<float>(kerning.x >> 6);
		}

		// Check if glyph exists for codepoint
		[[nodiscard]] bool HasGlyph(char32_t codepoint) const
		{
			if (!face_)
			{
				return false;
			}
			return FT_Get_Char_Index(face_, codepoint) != 0;
		}

		// Accessors
		[[nodiscard]] const FontInfo& GetInfo() const { return info_; }
		[[nodiscard]] float GetPixelSize() const { return currentPixelSize_; }
		[[nodiscard]] bool IsValid() const { return face_ != nullptr; }

		// Get number of faces in font file (for TTC/OTC collections)
		[[nodiscard]] std::int32_t GetFaceCount() const
		{
			return face_ ? static_cast<std::int32_t>(face_->num_faces) : 0;
		}

	private:
		void ExtractFontInfo()
		{
			if (!face_)
			{
				return;
			}

			info_.familyName = face_->family_name ? face_->family_name : "";
			info_.styleName  = face_->style_name ? face_->style_name : "";
			info_.fullName   = info_.familyName + " " + info_.styleName;

			// Parse style flags
			if (face_->style_flags & FT_STYLE_FLAG_ITALIC)
			{
				info_.style = FontStyle::Italic;
			}

			if (face_->style_flags & FT_STYLE_FLAG_BOLD)
			{
				info_.weight = FontWeight::Bold;
			}

			// Try to get more accurate weight from OS/2 table
			// This requires reading SFNT names which we'll keep simple for now

			// Check basic character coverage
			info_.hasBasicLatin = HasGlyph(U'A') && HasGlyph(U'a') && HasGlyph(U'0');
			info_.hasLatinExtended = HasGlyph(U'\u00E9') && HasGlyph(U'\u00F1');  // é, ñ
			info_.hasCyrillic = HasGlyph(U'\u042F');  // Я
			info_.hasGreek    = HasGlyph(U'\u03A9');  // Ω
			info_.hasCJK      = HasGlyph(U'\u4E2D');  // 中
		}

		FT_Face face_ = nullptr;
		std::vector<std::uint8_t> fontData_;  // Font data must persist for face lifetime
		FontInfo info_;
		float currentPixelSize_ = 0.0f;
	};

}
