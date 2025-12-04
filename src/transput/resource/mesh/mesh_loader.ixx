export module synodic.soul.transput:mesh_loader;

import std;
import synodic.soul.render.mesh;
import :mesh_error;

export namespace synodic::soul::mesh
{

	// Coordinate system documentation:
	// Soul Engine uses a right-handed coordinate system with +Y up:
	//   - +X: Right
	//   - +Y: Up
	//   - +Z: Forward (out of screen)
	// This matches glTF 2.0 specification, so no coordinate transformation is needed.

	// Load options for mesh importers
	struct MeshLoadOptions
	{
		// Compute tangents if not present in source data
		bool computeTangents = true;

		// Generate normals if not present (flat shading)
		bool generateNormals = true;

		// Flip texture V coordinate (some formats use top-left origin)
		bool flipTexCoordV = false;

		// Scale factor to apply to vertex positions
		float scale = 1.0f;

		// Future: Multi-primitive loading
		// Currently loads only the first primitive; set to true to load all
		// bool loadAllPrimitives = false;
	};

	// Loaded mesh result with metadata
	struct LoadedMesh
	{
		MeshData data;
		AABB bounds;

		// Source information
		std::string name;
		std::filesystem::path sourcePath;

		// Future: Support for multiple submeshes
		// std::vector<SubmeshInfo> submeshes;
	};

	// Abstract mesh loader interface
	// Implement this for each mesh format backend (GLTF, OBJ, FBX, etc.)
	class MeshLoader
	{
	public:
		MeshLoader()          = default;
		virtual ~MeshLoader() = default;

		MeshLoader(const MeshLoader&)            = delete;
		MeshLoader(MeshLoader&&) noexcept        = default;
		MeshLoader& operator=(const MeshLoader&) = delete;
		MeshLoader& operator=(MeshLoader&&) noexcept = default;

		// Check if this loader supports the given file extension
		[[nodiscard]] virtual bool SupportsExtension(std::string_view extension) const = 0;

		// Load mesh from file path
		[[nodiscard]] virtual MeshResult<LoadedMesh> Load(
			const std::filesystem::path& path,
			const MeshLoadOptions& options = {}) = 0;

		// Load mesh from memory buffer
		[[nodiscard]] virtual MeshResult<LoadedMesh> LoadFromMemory(
			std::span<const std::byte> data,
			std::string_view hint,  // File extension hint for format detection
			const MeshLoadOptions& options = {}) = 0;
	};

}

