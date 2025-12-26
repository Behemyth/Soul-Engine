import std;
import synodic.periapsis;
import synodic.soul.transput;
import synodic.soul.render.mesh;
import synodic.honesty.test;

using namespace honesty::test;
using namespace honesty::test::literals;
using namespace synodic::soul::gltf;
using namespace synodic::soul::mesh;

namespace
{
	// PBR layout constants (must match gltf_loader)
	constexpr std::size_t PBR_FLOATS_PER_VERTEX = 12;
	constexpr std::size_t PBR_POSITION_OFFSET = 0;

	// Helper to create minimal valid GLB data
	auto CreateMinimalGLB(std::string_view json) -> std::vector<std::byte>
	{
		std::vector<std::byte> data;

		// GLB Header (12 bytes)
		const std::uint32_t magic   = GLBMagic;
		const std::uint32_t version = GLBVersion;

		// JSON chunk (padded to 4-byte alignment)
		const std::size_t jsonPadded = (json.size() + 3) & ~3;
		const std::uint32_t totalLength =
			12 +                          // Header
			8 + static_cast<std::uint32_t>(jsonPadded);  // JSON chunk

		data.resize(totalLength);
		auto* ptr = data.data();

		// Write header
		std::memcpy(ptr, &magic, 4);
		ptr += 4;
		std::memcpy(ptr, &version, 4);
		ptr += 4;
		std::memcpy(ptr, &totalLength, 4);
		ptr += 4;

		// Write JSON chunk header
		const std::uint32_t jsonChunkLength = static_cast<std::uint32_t>(jsonPadded);
		const std::uint32_t jsonChunkType   = ChunkTypeJSON;
		std::memcpy(ptr, &jsonChunkLength, 4);
		ptr += 4;
		std::memcpy(ptr, &jsonChunkType, 4);
		ptr += 4;

		// Write JSON data (space-padded)
		std::memcpy(ptr, json.data(), json.size());
		for (std::size_t i = json.size(); i < jsonPadded; ++i)
		{
			ptr[i] = std::byte{0x20};  // Space padding per spec
		}

		return data;
	}

	Suite SUITE(
		"gltf_loader",
		[](const Fixture& fixture) -> Generator
		{
			co_yield "supports_extension"_test = [&](const Requirements& requirements)
			{
				GLTFLoader loader;

				requirements.Expect(loader.SupportsExtension(".gltf"));
				requirements.Expect(loader.SupportsExtension(".glb"));
				requirements.Expect(loader.SupportsExtension("gltf"));
				requirements.Expect(loader.SupportsExtension("glb"));
				requirements.Expect(!loader.SupportsExtension(".obj"));
				requirements.Expect(!loader.SupportsExtension(".fbx"));
			};

			co_yield "invalid_glb_header"_test = [&](const Requirements& requirements)
			{
				GLTFLoader loader;

				// Too small
				std::vector<std::byte> tooSmall(8);
				auto result = loader.LoadFromMemory(tooSmall, ".glb");
				requirements.Expect(!result.has_value());

				// Wrong magic
				std::vector<std::byte> wrongMagic(12);
				std::uint32_t badMagic = 0x12345678;
				std::memcpy(wrongMagic.data(), &badMagic, 4);
				result = loader.LoadFromMemory(wrongMagic, ".glb");
				requirements.Expect(!result.has_value());
			};

			co_yield "invalid_gltf_version"_test = [&](const Requirements& requirements)
			{
				GLTFLoader loader;

				// Version 1.0 not supported
				const std::string json = R"({"asset":{"version":"1.0"}})";
				auto glbData           = CreateMinimalGLB(json);
				auto result            = loader.LoadFromMemory(glbData, ".glb");

				requirements.Expect(!result.has_value());
			};

			co_yield "empty_mesh_error"_test = [&](const Requirements& requirements)
			{
				GLTFLoader loader;

				// Valid glTF 2.0 but no meshes
				const std::string json = R"({"asset":{"version":"2.0"}})";
				auto glbData           = CreateMinimalGLB(json);
				auto result            = loader.LoadFromMemory(glbData, ".glb");

				requirements.Expect(!result.has_value());
			};

			co_yield "load_options_defaults"_test = [&](const Requirements& requirements)
			{
				MeshLoadOptions options;

				requirements.Expect(options.computeTangents);
				requirements.Expect(options.generateNormals);
				requirements.Expect(!options.flipTexCoordV);
				requirements.ExpectEquals(options.scale, 1.0f);
			};

			co_yield "file_not_found"_test = [&](const Requirements& requirements)
			{
				GLTFLoader loader;

				auto result = loader.Load("nonexistent/path/mesh.glb");
				requirements.Expect(!result.has_value());
			};

			co_yield "load_box_glb"_test = [&](const Requirements& requirements)
			{
				const std::filesystem::path boxPath = "resources/assets/box/box.glb";

				if (!std::filesystem::exists(boxPath))
				{
					// Skip test if asset not present
					return;
				}

				auto result = LoadMesh(boxPath);
				requirements.Expect(result.has_value());

				if (result)
				{
					const auto& mesh = result->data;

					// Box should have vertex data and indices
					requirements.Expect(!mesh.vertexData.empty());
					requirements.Expect(!mesh.indices.empty());
					requirements.Expect(mesh.IsValid());
					requirements.Expect(mesh.vertexCount > 0);

				// Verify bounds are computed
					auto boundsMin = result->bounds.Min();
					auto boundsMax = result->bounds.Max();
					requirements.Expect(boundsMin.x <= boundsMax.x);
					requirements.Expect(boundsMin.y <= boundsMax.y);
					requirements.Expect(boundsMin.z <= boundsMax.z);
				}
			};

			co_yield "load_box_gltf"_test = [&](const Requirements& requirements)
			{
				const std::filesystem::path boxPath = "resources/assets/box/box.gltf";

				if (!std::filesystem::exists(boxPath))
				{
					// Skip test if asset not present
					return;
				}

				auto result = LoadMesh(boxPath);
				requirements.Expect(result.has_value());

				if (result)
				{
					requirements.Expect(result->data.IsValid());
				}
			};

			co_yield "scale_option"_test = [&](const Requirements& requirements)
			{
				const std::filesystem::path boxPath = "resources/assets/box/box.glb";

				if (!std::filesystem::exists(boxPath))
				{
					return;
				}

				// Load at scale 1.0
				MeshLoadOptions options1;
				options1.scale = 1.0f;
				auto result1   = LoadMesh(boxPath, options1);

				// Load at scale 2.0
				MeshLoadOptions options2;
				options2.scale = 2.0f;
				auto result2   = LoadMesh(boxPath, options2);

				if (result1 && result2 && result1->data.vertexCount > 0)
				{
				// Get first vertex's X position from raw float buffer
				const float pos1 = result1->data.vertexData[PBR_POSITION_OFFSET];
				const float pos2 = result2->data.vertexData[PBR_POSITION_OFFSET];

				// Scaled mesh should have positions 2x larger
				// Allow small epsilon for floating point
				requirements.Expect(std::abs(pos2 - pos1 * 2.0f) < 0.001f);
				}
			};
		});

	SuiteRegistrar _(SUITE);
}

