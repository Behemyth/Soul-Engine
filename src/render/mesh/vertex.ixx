export module synodic.soul.render.mesh:vertex;

import std;

// 3D vector type for mesh data
export struct vec3 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	constexpr vec3() = default;
	constexpr vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

	constexpr vec3 operator+(const vec3& other) const { return {x + other.x, y + other.y, z + other.z}; }
	constexpr vec3 operator-(const vec3& other) const { return {x - other.x, y - other.y, z - other.z}; }
	constexpr vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
	constexpr vec3 operator/(float s) const { return {x / s, y / s, z / s}; }

	constexpr vec3& operator+=(const vec3& other) { x += other.x; y += other.y; z += other.z; return *this; }
	constexpr vec3& operator-=(const vec3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
};

export constexpr vec3 operator*(float s, const vec3& v) { return v * s; }

export constexpr float Dot(const vec3& a, const vec3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

export constexpr vec3 Cross(const vec3& a, const vec3& b) {
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

export inline vec3 Normalize(const vec3& v) {
	float len = std::sqrt(Dot(v, v));
	return len > 0.0f ? v / len : vec3{};
}

// 2D vector type for texture coordinates
export struct vec2 {
	float x = 0.0f;
	float y = 0.0f;

	constexpr vec2() = default;
	constexpr vec2(float x_, float y_) : x(x_), y(y_) {}

	constexpr vec2 operator+(const vec2& other) const { return {x + other.x, y + other.y}; }
	constexpr vec2 operator-(const vec2& other) const { return {x - other.x, y - other.y}; }
	constexpr vec2 operator*(float s) const { return {x * s, y * s}; }
};

// 4D vector type for tangent (w = handedness)
export struct vec4 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

	constexpr vec4() = default;
	constexpr vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
	constexpr vec4(const vec3& v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
};

// PBR-ready vertex format
// Layout matches VkVertexInputAttributeDescription order
// Total size: 48 bytes (12 + 12 + 16 + 8)
export struct PBRVertex {
	vec3 position;    // location 0: RGB32_SFLOAT (12 bytes)
	vec3 normal;      // location 1: RGB32_SFLOAT (12 bytes)
	vec4 tangent;     // location 2: RGBA32_SFLOAT (16 bytes) - w is handedness for bitangent
	vec2 texCoord;    // location 3: RG32_SFLOAT (8 bytes)

	constexpr PBRVertex() = default;
	constexpr PBRVertex(vec3 pos, vec3 norm, vec4 tan, vec2 uv)
		: position(pos), normal(norm), tangent(tan), texCoord(uv) {}

	// Simplified constructor - tangent computed later
	constexpr PBRVertex(vec3 pos, vec3 norm, vec2 uv)
		: position(pos), normal(norm), tangent{0, 0, 0, 1}, texCoord(uv) {}
};

static_assert(sizeof(PBRVertex) == 48, "PBRVertex must be 48 bytes for GPU alignment");
// Note: offsetof checks removed - not available in C++23 modules without <cstddef>
// Layout is guaranteed by declaration order and standard layout rules:
// position: 0, normal: 12, tangent: 24, texCoord: 40

// Index type - 32-bit for large meshes (glTF compatibility)
export using Index = std::uint32_t;

// Compute tangents for a mesh using MikkTSpace algorithm (simplified)
// For a triangle with vertices v0, v1, v2 and UVs uv0, uv1, uv2:
// tangent = normalize((deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) / det)
export inline void ComputeTangent(
	PBRVertex& v0, PBRVertex& v1, PBRVertex& v2)
{
	vec3 edge1 = v1.position - v0.position;
	vec3 edge2 = v2.position - v0.position;

	vec2 deltaUV1 = v1.texCoord - v0.texCoord;
	vec2 deltaUV2 = v2.texCoord - v0.texCoord;

	float det = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;

	vec3 tangent;
	if (std::abs(det) < 1e-6f) {
		// Degenerate UV - use arbitrary tangent perpendicular to normal
		vec3 c1 = Cross(v0.normal, vec3{0, 0, 1});
		vec3 c2 = Cross(v0.normal, vec3{0, 1, 0});
		tangent = Dot(c1, c1) > Dot(c2, c2) ? c1 : c2;
	} else {
		float invDet = 1.0f / det;
		tangent = Normalize((edge1 * deltaUV2.y - edge2 * deltaUV1.y) * invDet);
	}

	// Compute handedness
	vec3 bitangent = Cross(v0.normal, tangent);
	float handedness = Dot(bitangent, Cross(v0.normal, tangent)) < 0.0f ? -1.0f : 1.0f;

	vec4 tangentWithHandedness{tangent, handedness};
	v0.tangent = tangentWithHandedness;
	v1.tangent = tangentWithHandedness;
	v2.tangent = tangentWithHandedness;
}
