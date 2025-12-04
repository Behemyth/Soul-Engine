export module synodic.soul.transput:mesh_error;

import std;

export namespace synodic::soul::mesh
{

	// Error codes for mesh loading operations
	enum class MeshErrorCode
	{
		Success = 0,

		// File errors
		FileNotFound,
		FileReadError,
		InvalidPath,

		// Format errors
		InvalidFormat,
		UnsupportedVersion,
		InvalidJson,
		InvalidBinary,

		// GLTF-specific errors
		InvalidGLBHeader,
		InvalidGLBChunk,
		MissingJsonChunk,
		MissingBinaryChunk,
		InvalidAccessor,
		InvalidBufferView,
		MissingAttribute,
		UnsupportedPrimitiveMode,
		UnsupportedComponentType,

		// Data errors
		EmptyMesh,
		InvalidVertexData,
		InvalidIndexData,
		OutOfBounds,
	};

	// Error category for mesh loading
	class MeshErrorCategory : public std::error_category
	{
	public:
		[[nodiscard]] const char* name() const noexcept override
		{
			return "mesh";
		}

		[[nodiscard]] std::string message(int ev) const override
		{
			switch (static_cast<MeshErrorCode>(ev))
			{
				case MeshErrorCode::Success:
					return "Success";
				case MeshErrorCode::FileNotFound:
					return "File not found";
				case MeshErrorCode::FileReadError:
					return "Failed to read file";
				case MeshErrorCode::InvalidPath:
					return "Invalid file path";
				case MeshErrorCode::InvalidFormat:
					return "Invalid mesh format";
				case MeshErrorCode::UnsupportedVersion:
					return "Unsupported format version";
				case MeshErrorCode::InvalidJson:
					return "Invalid JSON data";
				case MeshErrorCode::InvalidBinary:
					return "Invalid binary data";
				case MeshErrorCode::InvalidGLBHeader:
					return "Invalid GLB header";
				case MeshErrorCode::InvalidGLBChunk:
					return "Invalid GLB chunk";
				case MeshErrorCode::MissingJsonChunk:
					return "Missing JSON chunk in GLB";
				case MeshErrorCode::MissingBinaryChunk:
					return "Missing binary chunk in GLB";
				case MeshErrorCode::InvalidAccessor:
					return "Invalid accessor definition";
				case MeshErrorCode::InvalidBufferView:
					return "Invalid buffer view definition";
				case MeshErrorCode::MissingAttribute:
					return "Missing required vertex attribute";
				case MeshErrorCode::UnsupportedPrimitiveMode:
					return "Unsupported primitive mode (only triangles supported)";
				case MeshErrorCode::UnsupportedComponentType:
					return "Unsupported component type";
				case MeshErrorCode::EmptyMesh:
					return "Mesh contains no data";
				case MeshErrorCode::InvalidVertexData:
					return "Invalid vertex data";
				case MeshErrorCode::InvalidIndexData:
					return "Invalid index data";
				case MeshErrorCode::OutOfBounds:
					return "Data access out of bounds";
				default:
					return "Unknown mesh error";
			}
		}
	};

	// Get the singleton error category instance
	inline const MeshErrorCategory& GetMeshErrorCategory() noexcept
	{
		static MeshErrorCategory category;
		return category;
	}

	// Create an error code from a MeshErrorCode
	inline std::error_code MakeErrorCode(MeshErrorCode code) noexcept
	{
		return {static_cast<int>(code), GetMeshErrorCategory()};
	}

	// Result type for mesh operations
	template<typename T>
	using MeshResult = std::expected<T, std::error_code>;

	using MeshVoidResult = std::expected<void, std::error_code>;

}

// Enable automatic conversion to std::error_code
export template<>
struct std::is_error_code_enum<synodic::soul::mesh::MeshErrorCode> : std::true_type
{
};

export inline std::error_code make_error_code(synodic::soul::mesh::MeshErrorCode code) noexcept
{
	return synodic::soul::mesh::MakeErrorCode(code);
}

