export module synodic.soul.transput:font_loader;

import std;
import :font_types;
import :font_face;

export namespace synodic::soul::font
{

	// Font family: collection of faces with different weights/styles
	struct FontFamily
	{
		std::string name;
		std::vector<FontFace> faces;

		// Find best matching face for requested weight/style
		[[nodiscard]] FontFace* FindFace(
			FontWeight weight = FontWeight::Regular,
			FontStyle style   = FontStyle::Normal)
		{
			FontFace* bestMatch       = nullptr;
			int bestScore             = std::numeric_limits<int>::max();

			for (auto& face : faces)
			{
				const auto& info = face.GetInfo();
				int score        = 0;

				// Weight difference (0-800 range)
				score += std::abs(
					static_cast<int>(info.weight) - static_cast<int>(weight));

				// Style mismatch penalty
				if (info.style != style)
				{
					score += 1000;
				}

				if (score < bestScore)
				{
					bestScore = score;
					bestMatch = &face;
				}
			}

			return bestMatch;
		}

		// Find exact face
		[[nodiscard]] FontFace* FindExactFace(
			FontWeight weight,
			FontStyle style)
		{
			for (auto& face : faces)
			{
				const auto& info = face.GetInfo();
				if (info.weight == weight && info.style == style)
				{
					return &face;
				}
			}
			return nullptr;
		}
	};

	// Font loader: manages loading and caching of font faces
	class FontLoader
	{
	public:
		FontLoader() = default;

		// Load a single font face from file
		[[nodiscard]] FontResult<FontFace*> LoadFace(
			const std::filesystem::path& path,
			float pixelSize       = 16.0f,
			std::int32_t faceIndex = 0)
		{
			auto result = FontFace::LoadFromFile(path, faceIndex);
			if (!result)
			{
				return std::unexpected(result.error());
			}

			auto& face = *result;
			if (pixelSize > 0)
			{
				auto sizeResult = face.SetPixelSize(pixelSize);
				if (!sizeResult)
				{
					return std::unexpected(sizeResult.error());
				}
			}

			auto key      = MakeCacheKey(path, faceIndex);
			auto [it, _]  = loadedFaces_.emplace(key, std::move(face));
			return &it->second;
		}

		// Load entire font family from directory
		[[nodiscard]] FontResult<FontFamily*> LoadFamily(
			const std::filesystem::path& directory,
			std::string_view familyName,
			float pixelSize = 16.0f)
		{
			if (!std::filesystem::exists(directory))
			{
				return std::unexpected(FontError::FileNotFound);
			}

			FontFamily family;
			family.name = familyName;

			for (const auto& entry : std::filesystem::directory_iterator(directory))
			{
				if (!entry.is_regular_file())
				{
					continue;
				}

				auto ext = entry.path().extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(),
					[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

				if (ext != ".ttf" && ext != ".otf" && ext != ".ttc")
				{
					continue;
				}

				auto result = FontFace::LoadFromFile(entry.path());
				if (!result)
				{
					continue;  // Skip invalid fonts
				}

				auto& face = *result;

				// Check if this face belongs to the requested family
				if (!familyName.empty() &&
					face.GetInfo().familyName.find(familyName) == std::string::npos)
				{
					continue;
				}

				if (pixelSize > 0)
				{
					[[maybe_unused]] auto _ = face.SetPixelSize(pixelSize);
				}

				family.faces.push_back(std::move(face));
			}

			if (family.faces.empty())
			{
				return std::unexpected(FontError::FileNotFound);
			}

			auto [it, _] = loadedFamilies_.emplace(std::string(familyName), std::move(family));
			return &it->second;
		}

		// Get previously loaded family
		[[nodiscard]] FontFamily* GetFamily(std::string_view name)
		{
			auto it = loadedFamilies_.find(std::string(name));
			return it != loadedFamilies_.end() ? &it->second : nullptr;
		}

		// Clear all cached fonts
		void Clear()
		{
			loadedFaces_.clear();
			loadedFamilies_.clear();
		}

		// Get all loaded family names
		[[nodiscard]] std::vector<std::string_view> GetLoadedFamilies() const
		{
			std::vector<std::string_view> names;
			names.reserve(loadedFamilies_.size());
			for (const auto& [name, _] : loadedFamilies_)
			{
				names.push_back(name);
			}
			return names;
		}

	private:
		[[nodiscard]] static std::string MakeCacheKey(
			const std::filesystem::path& path,
			std::int32_t faceIndex)
		{
			return path.string() + ":" + std::to_string(faceIndex);
		}

		std::unordered_map<std::string, FontFace> loadedFaces_;
		std::unordered_map<std::string, FontFamily> loadedFamilies_;
	};

	// Default character sets for pre-rasterization
	namespace CharacterSets
	{
		// Basic ASCII printable characters (32-126)
		inline constexpr std::u32string_view BasicASCII =
			U" !\"#$%&'()*+,-./0123456789:;<=>?@"
			U"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
			U"abcdefghijklmnopqrstuvwxyz{|}~";

		// Extended Latin for Western European languages
		inline constexpr std::u32string_view LatinExtended =
			U"ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞß"
			U"àáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ"
			U"ŒœŠšŸŽž";

		// Common punctuation and symbols
		inline constexpr std::u32string_view CommonSymbols =
			U"€£¥¢©®™°±×÷…–—''""•·«»¿¡";
	}

}
