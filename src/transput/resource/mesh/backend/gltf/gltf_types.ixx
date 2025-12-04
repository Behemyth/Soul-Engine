module;
#include <glaze/glaze.hpp>

export module synodic.soul.transput:gltf_types;

import std;

// glTF 2.0 JSON Schema Types
// Reference: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
//
// Coordinate System:
// glTF uses a right-handed coordinate system with +Y up, which matches Soul Engine:
//   - +X: Right
//   - +Y: Up
//   - +Z: Forward (out of screen toward viewer)
// No coordinate transformation is required when importing glTF data.

export namespace synodic::soul::gltf
{

	// glTF Asset metadata
	struct Asset
	{
		std::string version;  // Required: glTF version (must be "2.0")
		std::optional<std::string> generator;
		std::optional<std::string> copyright;
		std::optional<std::string> minVersion;
	};

	// Buffer - raw binary data container
	struct Buffer
	{
		std::size_t byteLength = 0;  // Required
		std::optional<std::string> uri;  // Data URI or external file path
		std::optional<std::string> name;
	};

	// BufferView - a view into a buffer
	struct BufferView
	{
		std::size_t buffer = 0;      // Required: index into buffers array
		std::size_t byteLength = 0;  // Required
		std::size_t byteOffset = 0;  // Default: 0
		std::optional<std::size_t> byteStride;  // For vertex attributes (4-252, multiple of 4)
		std::optional<std::uint32_t> target;    // GL buffer target hint
		std::optional<std::string> name;
	};

	// Accessor component types (GL enum values)
	enum class ComponentType : std::uint32_t
	{
		Byte          = 5120,
		UnsignedByte  = 5121,
		Short         = 5122,
		UnsignedShort = 5123,
		UnsignedInt   = 5125,
		Float         = 5126,
	};

	// Accessor - typed view into buffer data
	struct Accessor
	{
		std::size_t bufferView = 0;       // Index into bufferViews (optional for sparse)
		std::size_t byteOffset = 0;       // Default: 0
		ComponentType componentType;      // Required
		bool normalized = false;          // Default: false
		std::size_t count = 0;            // Required: number of elements
		std::string type;                 // Required: "SCALAR", "VEC2", "VEC3", "VEC4", "MAT2", "MAT3", "MAT4"
		std::optional<std::vector<float>> min;
		std::optional<std::vector<float>> max;
		std::optional<std::string> name;
	};

	// Primitive modes
	enum class PrimitiveMode : std::uint32_t
	{
		Points        = 0,
		Lines         = 1,
		LineLoop      = 2,
		LineStrip     = 3,
		Triangles     = 4,
		TriangleStrip = 5,
		TriangleFan   = 6,
	};

	// Mesh Primitive - geometry with material
	struct Primitive
	{
		std::map<std::string, std::size_t> attributes;  // Required: attribute name -> accessor index
		std::optional<std::size_t> indices;             // Accessor index for indices
		std::optional<std::size_t> material;            // Material index
		PrimitiveMode mode = PrimitiveMode::Triangles;  // Default: triangles
	};

	// Mesh - collection of primitives
	struct Mesh
	{
		std::vector<Primitive> primitives;  // Required
		std::optional<std::string> name;
		std::optional<std::vector<float>> weights;  // Morph target weights
	};

	// Node - scene graph node
	struct Node
	{
		std::optional<std::size_t> mesh;
		std::optional<std::size_t> camera;
		std::optional<std::size_t> skin;
		std::optional<std::vector<std::size_t>> children;
		std::optional<std::string> name;

		// Transform (mutually exclusive: matrix OR TRS)
		std::optional<std::array<float, 16>> matrix;      // Column-major 4x4
		std::optional<std::array<float, 3>> translation;  // Default: [0,0,0]
		std::optional<std::array<float, 4>> rotation;     // Quaternion [x,y,z,w], default: [0,0,0,1]
		std::optional<std::array<float, 3>> scale;        // Default: [1,1,1]
	};

	// Scene - root of scene graph
	struct Scene
	{
		std::optional<std::vector<std::size_t>> nodes;  // Root node indices
		std::optional<std::string> name;
	};

	// PBR Metallic Roughness material model
	struct PbrMetallicRoughness
	{
		std::optional<std::array<float, 4>> baseColorFactor;  // Default: [1,1,1,1]
		std::optional<float> metallicFactor;                  // Default: 1
		std::optional<float> roughnessFactor;                 // Default: 1
		// TODO: baseColorTexture, metallicRoughnessTexture
	};

	// Material
	struct Material
	{
		std::optional<std::string> name;
		std::optional<PbrMetallicRoughness> pbrMetallicRoughness;
		std::optional<std::array<float, 3>> emissiveFactor;  // Default: [0,0,0]
		std::optional<std::string> alphaMode;                // "OPAQUE", "MASK", "BLEND"
		std::optional<float> alphaCutoff;                    // Default: 0.5
		std::optional<bool> doubleSided;                     // Default: false
	};

	// Root glTF document
	struct Document
	{
		Asset asset;  // Required
		std::optional<std::size_t> scene;  // Default scene index
		std::optional<std::vector<Scene>> scenes;
		std::optional<std::vector<Node>> nodes;
		std::optional<std::vector<Mesh>> meshes;
		std::optional<std::vector<Material>> materials;
		std::optional<std::vector<Accessor>> accessors;
		std::optional<std::vector<BufferView>> bufferViews;
		std::optional<std::vector<Buffer>> buffers;
	};

	// GLB chunk types
	constexpr std::uint32_t GLBMagic      = 0x46546C67;  // "glTF" in little-endian
	constexpr std::uint32_t GLBVersion    = 2;
	constexpr std::uint32_t ChunkTypeJSON = 0x4E4F534A;  // "JSON" in little-endian
	constexpr std::uint32_t ChunkTypeBIN  = 0x004E4942;  // "BIN\0" in little-endian

	// GLB header structure (12 bytes)
	struct GLBHeader
	{
		std::uint32_t magic;
		std::uint32_t version;
		std::uint32_t length;
	};

	// GLB chunk header (8 bytes)
	struct GLBChunkHeader
	{
		std::uint32_t chunkLength;
		std::uint32_t chunkType;
	};

}

// Glaze metadata for JSON parsing
template<>
struct glz::meta<synodic::soul::gltf::Asset>
{
	using T                     = synodic::soul::gltf::Asset;
	static constexpr auto value = object(
		"version", &T::version,
		"generator", &T::generator,
		"copyright", &T::copyright,
		"minVersion", &T::minVersion);
};

template<>
struct glz::meta<synodic::soul::gltf::Buffer>
{
	using T                     = synodic::soul::gltf::Buffer;
	static constexpr auto value = object(
		"byteLength", &T::byteLength,
		"uri", &T::uri,
		"name", &T::name);
};

template<>
struct glz::meta<synodic::soul::gltf::BufferView>
{
	using T                     = synodic::soul::gltf::BufferView;
	static constexpr auto value = object(
		"buffer", &T::buffer,
		"byteLength", &T::byteLength,
		"byteOffset", &T::byteOffset,
		"byteStride", &T::byteStride,
		"target", &T::target,
		"name", &T::name);
};

template<>
struct glz::meta<synodic::soul::gltf::Accessor>
{
	using T                     = synodic::soul::gltf::Accessor;
	static constexpr auto value = object(
		"bufferView", &T::bufferView,
		"byteOffset", &T::byteOffset,
		"componentType", &T::componentType,
		"normalized", &T::normalized,
		"count", &T::count,
		"type", &T::type,
		"min", &T::min,
		"max", &T::max,
		"name", &T::name);
};

template<>
struct glz::meta<synodic::soul::gltf::Primitive>
{
	using T                     = synodic::soul::gltf::Primitive;
	static constexpr auto value = object(
		"attributes", &T::attributes,
		"indices", &T::indices,
		"material", &T::material,
		"mode", &T::mode);
};

template<>
struct glz::meta<synodic::soul::gltf::Mesh>
{
	using T                     = synodic::soul::gltf::Mesh;
	static constexpr auto value = object(
		"primitives", &T::primitives,
		"name", &T::name,
		"weights", &T::weights);
};

template<>
struct glz::meta<synodic::soul::gltf::Node>
{
	using T                     = synodic::soul::gltf::Node;
	static constexpr auto value = object(
		"mesh", &T::mesh,
		"camera", &T::camera,
		"skin", &T::skin,
		"children", &T::children,
		"name", &T::name,
		"matrix", &T::matrix,
		"translation", &T::translation,
		"rotation", &T::rotation,
		"scale", &T::scale);
};

template<>
struct glz::meta<synodic::soul::gltf::Scene>
{
	using T                     = synodic::soul::gltf::Scene;
	static constexpr auto value = object(
		"nodes", &T::nodes,
		"name", &T::name);
};

template<>
struct glz::meta<synodic::soul::gltf::PbrMetallicRoughness>
{
	using T                     = synodic::soul::gltf::PbrMetallicRoughness;
	static constexpr auto value = object(
		"baseColorFactor", &T::baseColorFactor,
		"metallicFactor", &T::metallicFactor,
		"roughnessFactor", &T::roughnessFactor);
};

template<>
struct glz::meta<synodic::soul::gltf::Material>
{
	using T                     = synodic::soul::gltf::Material;
	static constexpr auto value = object(
		"name", &T::name,
		"pbrMetallicRoughness", &T::pbrMetallicRoughness,
		"emissiveFactor", &T::emissiveFactor,
		"alphaMode", &T::alphaMode,
		"alphaCutoff", &T::alphaCutoff,
		"doubleSided", &T::doubleSided);
};

template<>
struct glz::meta<synodic::soul::gltf::Document>
{
	using T                     = synodic::soul::gltf::Document;
	static constexpr auto value = object(
		"asset", &T::asset,
		"scene", &T::scene,
		"scenes", &T::scenes,
		"nodes", &T::nodes,
		"meshes", &T::meshes,
		"materials", &T::materials,
		"accessors", &T::accessors,
		"bufferViews", &T::bufferViews,
		"buffers", &T::buffers);
};

