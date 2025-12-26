export module synodic.soul.transput:font_types;

import std;

export namespace synodic::soul::font
{

	// Font weight enumeration matching CSS/OpenType weight classes
	enum class FontWeight : std::uint16_t
	{
		Thin       = 100,
		ExtraLight = 200,
		Light      = 300,
		Regular    = 400,
		Medium     = 500,
		SemiBold   = 600,
		Bold       = 700,
		ExtraBold  = 800,
		Black      = 900
	};

	// Font style/slant
	enum class FontStyle : std::uint8_t
	{
		Normal,
		Italic,
		Oblique
	};

	// Font stretch/width
	enum class FontStretch : std::uint8_t
	{
		UltraCondensed,
		ExtraCondensed,
		Condensed,
		SemiCondensed,
		Normal,
		SemiExpanded,
		Expanded,
		ExtraExpanded,
		UltraExpanded
	};

	// Metrics for a single glyph (in font units, typically 1/64th pixel at 26.6 fixed point)
	struct GlyphMetrics
	{
		std::uint32_t glyphIndex;  // Font-specific glyph index
		std::uint32_t codepoint;   // Unicode codepoint this glyph represents

		// Glyph dimensions in pixels (after rasterization)
		std::uint32_t width;
		std::uint32_t height;

		// Bearing: offset from cursor to top-left of glyph bitmap
		std::int32_t bearingX;
		std::int32_t bearingY;

		// Advance: how far to move cursor after rendering this glyph
		std::int32_t advanceX;
		std::int32_t advanceY;
	};

	// Rasterized glyph bitmap data
	struct GlyphBitmap
	{
		GlyphMetrics metrics;

		// Grayscale bitmap data (8-bit alpha coverage)
		// Row-major, top-to-bottom, with `pitch` bytes per row
		std::vector<std::uint8_t> data;
		std::uint32_t pitch;  // Bytes per row (may include padding)
	};

	// Font-level metrics (scaled to pixel size)
	struct FontMetrics
	{
		// Line metrics
		float ascender;     // Distance from baseline to top of line
		float descender;    // Distance from baseline to bottom (typically negative)
		float lineHeight;   // Recommended line spacing (ascender - descender + lineGap)
		float lineGap;      // Additional spacing between lines

		// Em square size
		float unitsPerEm;

		// Underline/strikeout positioning
		float underlinePosition;
		float underlineThickness;
		float strikeoutPosition;
		float strikeoutThickness;

		// Maximum glyph extents
		float maxAdvanceWidth;
		float maxAdvanceHeight;
	};

	// Font identification
	struct FontInfo
	{
		std::string familyName;    // e.g., "Roboto"
		std::string styleName;     // e.g., "Bold Italic"
		std::string fullName;      // e.g., "Roboto Bold Italic"
		std::string postScriptName;

		FontWeight weight   = FontWeight::Regular;
		FontStyle style     = FontStyle::Normal;
		FontStretch stretch = FontStretch::Normal;

		// Supported Unicode ranges (simplified)
		bool hasBasicLatin    = false;
		bool hasLatinExtended = false;
		bool hasCyrillic      = false;
		bool hasGreek         = false;
		bool hasCJK           = false;
	};

	// Request for loading a font with specific parameters
	struct FontLoadRequest
	{
		std::filesystem::path path;

		// Pixel size for rasterization (0 = load outline only)
		float pixelSize = 16.0f;

		// DPI for scaling (typically 72 for pixels, 96 for Windows)
		std::uint32_t dpiX = 96;
		std::uint32_t dpiY = 96;

		// Character set to pre-rasterize (empty = on-demand)
		std::u32string preloadCharacters;

		// Face index for font collections (TTC/OTC files)
		std::int32_t faceIndex = 0;
	};

	// Error types for font operations
	enum class FontError
	{
		None,
		FileNotFound,
		InvalidFormat,
		UnsupportedFormat,
		FreeTypeError,
		GlyphNotFound,
		OutOfMemory,
		InvalidSize
	};

	// Result type for font operations
	template <typename T>
	using FontResult = std::expected<T, FontError>;

}
