export module synodic.soul.render.mesh:vertex;

import std;

// Vertex attribute format - matches Vulkan/GPU formats
export enum class VertexAttributeFormat : std::uint8_t {
	Float,       // R32_SFLOAT
	Float2,      // R32G32_SFLOAT
	Float3,      // R32G32B32_SFLOAT
	Float4,      // R32G32B32A32_SFLOAT
};

// Returns the byte size of a vertex attribute format
export [[nodiscard]] constexpr std::size_t AttributeFormatSize(VertexAttributeFormat format) noexcept {
	switch (format) {
		case VertexAttributeFormat::Float:  return 4;
		case VertexAttributeFormat::Float2: return 8;
		case VertexAttributeFormat::Float3: return 12;
		case VertexAttributeFormat::Float4: return 16;
	}
	return 0;
}

// Describes a single vertex attribute within a vertex layout
export struct VertexAttribute {
	std::uint32_t location;           // Shader location binding
	VertexAttributeFormat format;     // Data format
	std::uint32_t offset;             // Byte offset within vertex

	constexpr VertexAttribute() = default;
	constexpr VertexAttribute(std::uint32_t loc, VertexAttributeFormat fmt, std::uint32_t off)
		: location(loc), format(fmt), offset(off) {}
};

// Describes the layout of vertex data in a buffer
export struct VertexLayout {
	std::vector<VertexAttribute> attributes;
	std::uint32_t stride = 0;  // Total bytes per vertex

	VertexLayout() = default;

	// Build layout from a list of formats (computes offsets automatically)
	static VertexLayout Build(std::initializer_list<VertexAttributeFormat> formats) {
		VertexLayout layout;
		std::uint32_t offset = 0;
		std::uint32_t location = 0;

		for (auto format : formats) {
			layout.attributes.emplace_back(location++, format, offset);
			offset += static_cast<std::uint32_t>(AttributeFormatSize(format));
		}

		layout.stride = offset;
		return layout;
	}

	[[nodiscard]] std::size_t AttributeCount() const noexcept { return attributes.size(); }
};

// Standard PBR vertex layout: position(vec3) + normal(vec3) + tangent(vec4) + texcoord(vec2)
// Total stride: 12 + 12 + 16 + 8 = 48 bytes
export inline VertexLayout PBRVertexLayout() {
	return VertexLayout::Build({
		VertexAttributeFormat::Float3,  // position  (location 0, offset 0)
		VertexAttributeFormat::Float3,  // normal    (location 1, offset 12)
		VertexAttributeFormat::Float4,  // tangent   (location 2, offset 24)
		VertexAttributeFormat::Float2,  // texcoord  (location 3, offset 40)
	});
}

// Index type - 32-bit for large meshes (glTF compatibility)
export using Index = std::uint32_t;
