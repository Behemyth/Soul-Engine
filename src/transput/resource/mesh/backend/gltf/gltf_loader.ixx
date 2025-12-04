module;
#include <glaze/glaze.hpp>

export module synodic.soul.transput:gltf_loader;

import std;
import synodic.soul.render.mesh;
import :mesh_error;
import :mesh_loader;
import :gltf_types;

export namespace synodic::soul::gltf
{

	// GLTF Mesh Loader Backend
	// Supports both .gltf (JSON + external/embedded buffers) and .glb (binary container)
	//
	// Coordinate System:
	// glTF uses a right-handed coordinate system with +Y up, matching Soul Engine.
	// No coordinate transformation is applied during import.
	//
	// Current Limitations (Future improvements):
	// - Only loads the first mesh and first primitive
	// - Does not load material textures
	// - Does not support morph targets or skinning
	// - Does not process scene hierarchy/transforms
	// - External .bin files not yet supported (use GLB or embedded base64)

	class GLTFLoader : public mesh::MeshLoader
	{
	public:
		GLTFLoader()           = default;
		~GLTFLoader() override = default;

		[[nodiscard]] bool SupportsExtension(std::string_view extension) const override
		{
			return extension == ".gltf" || extension == ".glb" ||
			       extension == "gltf" || extension == "glb";
		}

		[[nodiscard]] mesh::MeshResult<mesh::LoadedMesh> Load(
			const std::filesystem::path& path,
			const mesh::MeshLoadOptions& options = {}) override
		{
			// Read file into memory
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (!file)
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::FileNotFound));
			}

			const auto fileSize = file.tellg();
			file.seekg(0, std::ios::beg);

			std::vector<std::byte> fileData(static_cast<std::size_t>(fileSize));
			if (!file.read(reinterpret_cast<char*>(fileData.data()), fileSize))
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::FileReadError));
			}

			auto result = LoadFromMemory(fileData, path.extension().string(), options);
			if (result)
			{
				result->sourcePath = path;
			}
			return result;
		}

		[[nodiscard]] mesh::MeshResult<mesh::LoadedMesh> LoadFromMemory(
			std::span<const std::byte> data,
			std::string_view hint,
			const mesh::MeshLoadOptions& options = {}) override
		{
			if (data.size() < 4)
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidFormat));
			}

			// Check for GLB magic number
			const std::uint32_t magic = *reinterpret_cast<const std::uint32_t*>(data.data());
			if (magic == GLBMagic)
			{
				return LoadGLB(data, options);
			}

			// Assume JSON (.gltf)
			return LoadGLTFJson(data, {}, options);
		}

	private:
		// Load GLB binary container format
		[[nodiscard]] mesh::MeshResult<mesh::LoadedMesh> LoadGLB(
			std::span<const std::byte> data,
			const mesh::MeshLoadOptions& options)
		{
			if (data.size() < sizeof(GLBHeader))
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidGLBHeader));
			}

			// Parse header
			const auto& header = *reinterpret_cast<const GLBHeader*>(data.data());
			if (header.magic != GLBMagic)
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidGLBHeader));
			}
			if (header.version != GLBVersion)
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::UnsupportedVersion));
			}
			if (header.length > data.size())
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidGLBHeader));
			}

			// Parse chunks
			std::span<const std::byte> jsonChunk;
			std::span<const std::byte> binChunk;

			std::size_t offset = sizeof(GLBHeader);
			while (offset + sizeof(GLBChunkHeader) <= data.size())
			{
				const auto& chunkHeader = *reinterpret_cast<const GLBChunkHeader*>(data.data() + offset);
				offset += sizeof(GLBChunkHeader);

				if (offset + chunkHeader.chunkLength > data.size())
				{
					return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidGLBChunk));
				}

				const auto chunkData = data.subspan(offset, chunkHeader.chunkLength);

				if (chunkHeader.chunkType == ChunkTypeJSON)
				{
					jsonChunk = chunkData;
				}
				else if (chunkHeader.chunkType == ChunkTypeBIN)
				{
					binChunk = chunkData;
				}

				offset += chunkHeader.chunkLength;
			}

			if (jsonChunk.empty())
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::MissingJsonChunk));
			}

			return LoadGLTFJson(jsonChunk, binChunk, options);
		}

		// Load GLTF JSON format (with optional embedded binary data)
		[[nodiscard]] mesh::MeshResult<mesh::LoadedMesh> LoadGLTFJson(
			std::span<const std::byte> jsonData,
			std::span<const std::byte> binaryData,
			const mesh::MeshLoadOptions& options)
		{
			// Parse JSON
			std::string_view jsonStr(reinterpret_cast<const char*>(jsonData.data()), jsonData.size());

			Document doc;
			const auto parseError = glz::read_json(doc, jsonStr);
			if (parseError)
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidJson));
			}

			// Validate glTF version
			if (doc.asset.version.empty() || doc.asset.version[0] != '2')
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::UnsupportedVersion));
			}

			// Get buffer data (either from GLB binary chunk or embedded base64)
			std::vector<std::vector<std::byte>> bufferData;
			if (auto result = LoadBuffers(doc, binaryData); result)
			{
				bufferData = std::move(*result);
			}
			else
			{
				return std::unexpected(result.error());
			}

			// Find first mesh
			if (!doc.meshes || doc.meshes->empty())
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::EmptyMesh));
			}

			const auto& mesh = (*doc.meshes)[0];
			if (mesh.primitives.empty())
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::EmptyMesh));
			}

			// Load first primitive (future: support multiple primitives as submeshes)
			const auto& primitive = mesh.primitives[0];
			if (primitive.mode != PrimitiveMode::Triangles)
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::UnsupportedPrimitiveMode));
			}

			return LoadPrimitive(doc, primitive, bufferData, options, mesh.name.value_or(""));
		}

		// Load buffer data from GLB binary chunk or embedded base64 URIs
		[[nodiscard]] mesh::MeshResult<std::vector<std::vector<std::byte>>> LoadBuffers(
			const Document& doc,
			std::span<const std::byte> glbBinaryChunk)
		{
			std::vector<std::vector<std::byte>> buffers;

			if (!doc.buffers)
			{
				return buffers;
			}

			for (std::size_t i = 0; i < doc.buffers->size(); ++i)
			{
				const auto& buffer = (*doc.buffers)[i];

				if (!buffer.uri)
				{
					// No URI - use GLB binary chunk (buffer 0)
					if (i == 0 && !glbBinaryChunk.empty())
					{
						buffers.emplace_back(glbBinaryChunk.begin(), glbBinaryChunk.end());
					}
					else
					{
						return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::MissingBinaryChunk));
					}
				}
				else if (buffer.uri->starts_with("data:"))
				{
					// Base64 embedded data
					auto decoded = DecodeBase64DataUri(*buffer.uri);
					if (!decoded)
					{
						return std::unexpected(decoded.error());
					}
					buffers.push_back(std::move(*decoded));
				}
				else
				{
					// External file - not yet supported
					// Future: Load external .bin files relative to .gltf path
					return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidFormat));
				}
			}

			return buffers;
		}

		// Decode base64 data URI
		[[nodiscard]] mesh::MeshResult<std::vector<std::byte>> DecodeBase64DataUri(std::string_view uri)
		{
			// Format: data:[<mediatype>][;base64],<data>
			const auto commaPos = uri.find(',');
			if (commaPos == std::string_view::npos)
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidFormat));
			}

			const auto base64Data = uri.substr(commaPos + 1);
			return DecodeBase64(base64Data);
		}

		// Simple base64 decoder
		[[nodiscard]] mesh::MeshResult<std::vector<std::byte>> DecodeBase64(std::string_view encoded)
		{
			static constexpr std::array<std::int8_t, 256> decodeTable = []()
			{
				std::array<std::int8_t, 256> table{};
				std::fill(table.begin(), table.end(), -1);
				for (std::size_t i = 0; i < 26; ++i)
				{
					table['A' + i] = static_cast<std::int8_t>(i);
					table['a' + i] = static_cast<std::int8_t>(i + 26);
				}
				for (std::size_t i = 0; i < 10; ++i)
				{
					table['0' + i] = static_cast<std::int8_t>(i + 52);
				}
				table['+'] = 62;
				table['/'] = 63;
				table['='] = 0;  // Padding
				return table;
			}();

			std::vector<std::byte> decoded;
			decoded.reserve((encoded.size() * 3) / 4);

			std::uint32_t buffer = 0;
			std::size_t bits     = 0;

			for (char c : encoded)
			{
				if (c == '=' || std::isspace(static_cast<unsigned char>(c)))
				{
					continue;
				}

				const auto value = decodeTable[static_cast<unsigned char>(c)];
				if (value < 0)
				{
					return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidFormat));
				}

				buffer = (buffer << 6) | static_cast<std::uint32_t>(value);
				bits += 6;

				if (bits >= 8)
				{
					bits -= 8;
					decoded.push_back(static_cast<std::byte>((buffer >> bits) & 0xFF));
				}
			}

			return decoded;
		}

		// Load a single primitive into MeshData
		[[nodiscard]] mesh::MeshResult<mesh::LoadedMesh> LoadPrimitive(
			const Document& doc,
			const Primitive& primitive,
			const std::vector<std::vector<std::byte>>& bufferData,
			const mesh::MeshLoadOptions& options,
			const std::string& meshName)
		{
			mesh::LoadedMesh result;
			result.name = meshName;

			// Get required POSITION attribute
			auto posIt = primitive.attributes.find("POSITION");
			if (posIt == primitive.attributes.end())
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::MissingAttribute));
			}

			const std::size_t posAccessorIndex = posIt->second;
			if (!doc.accessors || posAccessorIndex >= doc.accessors->size())
			{
				return std::unexpected(mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidAccessor));
			}

			const auto& posAccessor = (*doc.accessors)[posAccessorIndex];
			const std::size_t vertexCount = posAccessor.count;

			// Pre-allocate vertices
			result.data.vertices.resize(vertexCount);

			// Read positions
			if (auto err = ReadAttribute<vec3>(doc, bufferData, posAccessorIndex,
					[&](std::size_t i, const vec3& v) { result.data.vertices[i].position = v * options.scale; }))
			{
				return std::unexpected(*err);
			}

			// Read normals (optional)
			bool hasNormals = false;
			if (auto normIt = primitive.attributes.find("NORMAL"); normIt != primitive.attributes.end())
			{
				if (auto err = ReadAttribute<vec3>(doc, bufferData, normIt->second,
						[&](std::size_t i, const vec3& v) { result.data.vertices[i].normal = v; }))
				{
					return std::unexpected(*err);
				}
				hasNormals = true;
			}

			// Read tangents (optional)
			bool hasTangents = false;
			if (auto tanIt = primitive.attributes.find("TANGENT"); tanIt != primitive.attributes.end())
			{
				if (auto err = ReadAttribute<vec4>(doc, bufferData, tanIt->second,
						[&](std::size_t i, const vec4& v) { result.data.vertices[i].tangent = v; }))
				{
					return std::unexpected(*err);
				}
				hasTangents = true;
			}

			// Read texture coordinates (optional)
			if (auto uvIt = primitive.attributes.find("TEXCOORD_0"); uvIt != primitive.attributes.end())
			{
				if (auto err = ReadAttribute<vec2>(doc, bufferData, uvIt->second,
						[&](std::size_t i, const vec2& v)
						{
							result.data.vertices[i].texCoord = options.flipTexCoordV ? vec2{v.x, 1.0f - v.y} : v;
						}))
				{
					return std::unexpected(*err);
				}
			}

			// Read indices (optional - if not present, use sequential indices)
			if (primitive.indices)
			{
				if (auto err = ReadIndices(doc, bufferData, *primitive.indices, result.data.indices))
				{
					return std::unexpected(*err);
				}
			}
			else
			{
				// Generate sequential indices
				result.data.indices.resize(vertexCount);
				for (std::size_t i = 0; i < vertexCount; ++i)
				{
					result.data.indices[i] = static_cast<Index>(i);
				}
			}

			// Generate normals if missing and requested
			if (!hasNormals && options.generateNormals)
			{
				GenerateFlatNormals(result.data);
			}

			// Compute tangents if missing and requested
			if (!hasTangents && options.computeTangents)
			{
				result.data.ComputeTangents();
			}

			// Compute bounds
			result.bounds = ComputeAABB(result.data);

			return result;
		}

		// Read an attribute from accessor into vertices
		template<typename T, typename Callback>
		[[nodiscard]] std::optional<std::error_code> ReadAttribute(
			const Document& doc,
			const std::vector<std::vector<std::byte>>& bufferData,
			std::size_t accessorIndex,
			Callback&& callback)
		{
			if (!doc.accessors || accessorIndex >= doc.accessors->size())
			{
				return mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidAccessor);
			}

			const auto& accessor = (*doc.accessors)[accessorIndex];

			if (!doc.bufferViews || accessor.bufferView >= doc.bufferViews->size())
			{
				return mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidBufferView);
			}

			const auto& bufferView = (*doc.bufferViews)[accessor.bufferView];

			if (bufferView.buffer >= bufferData.size())
			{
				return mesh::MakeErrorCode(mesh::MeshErrorCode::OutOfBounds);
			}

			const auto& buffer = bufferData[bufferView.buffer];
			const std::size_t byteOffset = bufferView.byteOffset + accessor.byteOffset;
			const std::size_t byteStride = bufferView.byteStride.value_or(sizeof(T));

			if (byteOffset + accessor.count * byteStride > buffer.size())
			{
				return mesh::MakeErrorCode(mesh::MeshErrorCode::OutOfBounds);
			}

			// Read data
			const std::byte* ptr = buffer.data() + byteOffset;
			for (std::size_t i = 0; i < accessor.count; ++i)
			{
				T value;
				std::memcpy(&value, ptr, sizeof(T));
				callback(i, value);
				ptr += byteStride;
			}

			return std::nullopt;
		}

		// Read indices from accessor
		[[nodiscard]] std::optional<std::error_code> ReadIndices(
			const Document& doc,
			const std::vector<std::vector<std::byte>>& bufferData,
			std::size_t accessorIndex,
			std::vector<Index>& indices)
		{
			if (!doc.accessors || accessorIndex >= doc.accessors->size())
			{
				return mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidAccessor);
			}

			const auto& accessor = (*doc.accessors)[accessorIndex];

			if (!doc.bufferViews || accessor.bufferView >= doc.bufferViews->size())
			{
				return mesh::MakeErrorCode(mesh::MeshErrorCode::InvalidBufferView);
			}

			const auto& bufferView = (*doc.bufferViews)[accessor.bufferView];

			if (bufferView.buffer >= bufferData.size())
			{
				return mesh::MakeErrorCode(mesh::MeshErrorCode::OutOfBounds);
			}

			const auto& buffer = bufferData[bufferView.buffer];
			const std::size_t byteOffset = bufferView.byteOffset + accessor.byteOffset;

			indices.resize(accessor.count);

			const std::byte* ptr = buffer.data() + byteOffset;

			switch (accessor.componentType)
			{
				case ComponentType::UnsignedByte:
					for (std::size_t i = 0; i < accessor.count; ++i)
					{
						indices[i] = static_cast<Index>(*reinterpret_cast<const std::uint8_t*>(ptr));
						ptr += sizeof(std::uint8_t);
					}
					break;

				case ComponentType::UnsignedShort:
					for (std::size_t i = 0; i < accessor.count; ++i)
					{
						indices[i] = static_cast<Index>(*reinterpret_cast<const std::uint16_t*>(ptr));
						ptr += sizeof(std::uint16_t);
					}
					break;

				case ComponentType::UnsignedInt:
					for (std::size_t i = 0; i < accessor.count; ++i)
					{
						indices[i] = *reinterpret_cast<const std::uint32_t*>(ptr);
						ptr += sizeof(std::uint32_t);
					}
					break;

				default:
					return mesh::MakeErrorCode(mesh::MeshErrorCode::UnsupportedComponentType);
			}

			return std::nullopt;
		}

		// Generate flat normals for a mesh without normals
		void GenerateFlatNormals(MeshData& mesh)
		{
			for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
			{
				auto& v0 = mesh.vertices[mesh.indices[i]];
				auto& v1 = mesh.vertices[mesh.indices[i + 1]];
				auto& v2 = mesh.vertices[mesh.indices[i + 2]];

				const vec3 edge1 = v1.position - v0.position;
				const vec3 edge2 = v2.position - v0.position;
				const vec3 normal = Normalize(Cross(edge1, edge2));

				v0.normal = normal;
				v1.normal = normal;
				v2.normal = normal;
			}
		}
	};

	// Convenience function to load a GLTF/GLB file
	[[nodiscard]] inline mesh::MeshResult<mesh::LoadedMesh> LoadMesh(
		const std::filesystem::path& path,
		const mesh::MeshLoadOptions& options = {})
	{
		GLTFLoader loader;
		return loader.Load(path, options);
	}

}

