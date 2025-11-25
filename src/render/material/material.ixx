export module synodic.soul.render.material;

import std;
import synodic.soul.core;

// 4x4 matrix for transforms - column-major for GPU compatibility
export struct mat4 {
	float data[16] = {
		1, 0, 0, 0,  // column 0
		0, 1, 0, 0,  // column 1
		0, 0, 1, 0,  // column 2
		0, 0, 0, 1   // column 3
	};

	constexpr mat4() = default;

	// Access element at (row, col)
	constexpr float& operator()(int row, int col) { return data[col * 4 + row]; }
	constexpr float operator()(int row, int col) const { return data[col * 4 + row]; }

	// Identity matrix
	[[nodiscard]] static constexpr mat4 Identity() { return mat4{}; }

	// Translation matrix
	[[nodiscard]] static constexpr mat4 Translation(float x, float y, float z) {
		mat4 m;
		m(0, 3) = x;
		m(1, 3) = y;
		m(2, 3) = z;
		return m;
	}

	// Uniform scale matrix
	[[nodiscard]] static constexpr mat4 Scale(float s) {
		mat4 m;
		m(0, 0) = s;
		m(1, 1) = s;
		m(2, 2) = s;
		return m;
	}

	// Non-uniform scale matrix
	[[nodiscard]] static constexpr mat4 Scale(float x, float y, float z) {
		mat4 m;
		m(0, 0) = x;
		m(1, 1) = y;
		m(2, 2) = z;
		return m;
	}

	// Rotation around X axis
	[[nodiscard]] static mat4 RotationX(float radians) {
		mat4 m;
		float c = std::cos(radians);
		float s = std::sin(radians);
		m(1, 1) = c;
		m(1, 2) = -s;
		m(2, 1) = s;
		m(2, 2) = c;
		return m;
	}

	// Rotation around Y axis
	[[nodiscard]] static mat4 RotationY(float radians) {
		mat4 m;
		float c = std::cos(radians);
		float s = std::sin(radians);
		m(0, 0) = c;
		m(0, 2) = s;
		m(2, 0) = -s;
		m(2, 2) = c;
		return m;
	}

	// Rotation around Z axis
	[[nodiscard]] static mat4 RotationZ(float radians) {
		mat4 m;
		float c = std::cos(radians);
		float s = std::sin(radians);
		m(0, 0) = c;
		m(0, 1) = -s;
		m(1, 0) = s;
		m(1, 1) = c;
		return m;
	}

	// Perspective projection (reversed-Z for better depth precision)
	[[nodiscard]] static mat4 Perspective(float fovYRadians, float aspect, float nearPlane, float farPlane) {
		mat4 m{};
		float tanHalfFov = std::tan(fovYRadians * 0.5f);

		m(0, 0) = 1.0f / (aspect * tanHalfFov);
		m(1, 1) = 1.0f / tanHalfFov;

		// Reversed-Z: map near to 1, far to 0
		m(2, 2) = nearPlane / (farPlane - nearPlane);
		m(2, 3) = (farPlane * nearPlane) / (farPlane - nearPlane);
		m(3, 2) = -1.0f;
		m(3, 3) = 0.0f;

		return m;
	}

	// Look-at view matrix
	[[nodiscard]] static mat4 LookAt(
		float eyeX, float eyeY, float eyeZ,
		float targetX, float targetY, float targetZ,
		float upX, float upY, float upZ)
	{
		// Forward = normalize(target - eye)
		float fx = targetX - eyeX;
		float fy = targetY - eyeY;
		float fz = targetZ - eyeZ;
		float flen = std::sqrt(fx * fx + fy * fy + fz * fz);
		fx /= flen; fy /= flen; fz /= flen;

		// Right = normalize(forward x up)
		float rx = fy * upZ - fz * upY;
		float ry = fz * upX - fx * upZ;
		float rz = fx * upY - fy * upX;
		float rlen = std::sqrt(rx * rx + ry * ry + rz * rz);
		rx /= rlen; ry /= rlen; rz /= rlen;

		// Up = right x forward
		float ux = ry * fz - rz * fy;
		float uy = rz * fx - rx * fz;
		float uz = rx * fy - ry * fx;

		mat4 m;
		m(0, 0) = rx;  m(0, 1) = ry;  m(0, 2) = rz;  m(0, 3) = -(rx * eyeX + ry * eyeY + rz * eyeZ);
		m(1, 0) = ux;  m(1, 1) = uy;  m(1, 2) = uz;  m(1, 3) = -(ux * eyeX + uy * eyeY + uz * eyeZ);
		m(2, 0) = -fx; m(2, 1) = -fy; m(2, 2) = -fz; m(2, 3) = (fx * eyeX + fy * eyeY + fz * eyeZ);
		m(3, 0) = 0;   m(3, 1) = 0;   m(3, 2) = 0;   m(3, 3) = 1;

		return m;
	}
};

// Matrix multiplication
export constexpr mat4 operator*(const mat4& a, const mat4& b) {
	mat4 result{};
	for (int col = 0; col < 4; ++col) {
		for (int row = 0; row < 4; ++row) {
			result(row, col) = 0;
			for (int k = 0; k < 4; ++k) {
				result(row, col) += a(row, k) * b(k, col);
			}
		}
	}
	return result;
}

static_assert(sizeof(mat4) == 64, "mat4 must be 64 bytes");

// PBR material parameters
// Aligned for GPU uniform buffer (std140 layout)
export struct PBRMaterialData {
	// Base color (linear RGB) + alpha
	float baseColorR = 1.0f;
	float baseColorG = 1.0f;
	float baseColorB = 1.0f;
	float baseColorA = 1.0f;

	// Metallic-roughness
	float metallic = 0.0f;   // 0 = dielectric, 1 = metal
	float roughness = 0.5f;  // 0 = smooth, 1 = rough
	float ao = 1.0f;         // Ambient occlusion
	float _padding0 = 0.0f;

	// Emissive (linear RGB) + intensity
	float emissiveR = 0.0f;
	float emissiveG = 0.0f;
	float emissiveB = 0.0f;
	float emissiveIntensity = 1.0f;
};

static_assert(sizeof(PBRMaterialData) == 48, "PBRMaterialData must be 48 bytes");

// Per-object transform data for push constants
// Push constant limit is typically 128 bytes, so we use 192 bytes via uniform
export struct TransformData {
	mat4 model;          // 64 bytes - object to world
	mat4 viewProjection; // 64 bytes - world to clip
	mat4 normalMatrix;   // 64 bytes - for transforming normals (inverse transpose of model)
};

static_assert(sizeof(TransformData) == 192, "TransformData must be 192 bytes");

// Compact push constant data (fits in 128 bytes)
export struct PushConstantData {
	mat4 mvp;           // 64 bytes - model-view-projection
	mat4 model;         // 64 bytes - model matrix for lighting
};

static_assert(sizeof(PushConstantData) == 128, "PushConstantData must be 128 bytes");

// Light types
export enum class LightType : std::uint32_t {
	Directional = 0,
	Point = 1,
	Spot = 2
};

// Light data for GPU (std140)
export struct LightData {
	// Position (point/spot) or direction (directional)
	float positionX = 0.0f;
	float positionY = 0.0f;
	float positionZ = 0.0f;
	float range = 10.0f;  // Attenuation range for point/spot

	// Direction (for spot lights)
	float directionX = 0.0f;
	float directionY = -1.0f;
	float directionZ = 0.0f;
	float spotAngle = 0.785398f;  // 45 degrees in radians

	// Color and intensity
	float colorR = 1.0f;
	float colorG = 1.0f;
	float colorB = 1.0f;
	float intensity = 1.0f;

	// Type and flags
	LightType type = LightType::Directional;
	std::uint32_t castShadows = 0;
	float _padding[2] = {0, 0};
};

static_assert(sizeof(LightData) == 64, "LightData must be 64 bytes");

// Scene-wide lighting data
export struct SceneLightingData {
	// Camera position for specular
	float cameraPositionX = 0.0f;
	float cameraPositionY = 0.0f;
	float cameraPositionZ = 0.0f;
	std::uint32_t lightCount = 0;

	// Ambient/environment
	float ambientColorR = 0.03f;
	float ambientColorG = 0.03f;
	float ambientColorB = 0.03f;
	float ambientIntensity = 1.0f;

	// Up to 4 lights inline (more via SSBO later)
	LightData lights[4];
};

static_assert(sizeof(SceneLightingData) == 32 + 64 * 4, "SceneLightingData size mismatch");

// Material handle - references a material in the material system
export struct MaterialHandle {
	Entity entity;
	std::uint32_t index = 0;

	[[nodiscard]] bool IsValid() const noexcept {
		return index != std::numeric_limits<std::uint32_t>::max();
	}
};

// Default materials
export inline PBRMaterialData DefaultMaterial() {
	return PBRMaterialData{};
}

export inline PBRMaterialData MetallicMaterial(float r, float g, float b) {
	PBRMaterialData mat;
	mat.baseColorR = r;
	mat.baseColorG = g;
	mat.baseColorB = b;
	mat.metallic = 1.0f;
	mat.roughness = 0.3f;
	return mat;
}

export inline PBRMaterialData DielectricMaterial(float r, float g, float b, float roughness = 0.5f) {
	PBRMaterialData mat;
	mat.baseColorR = r;
	mat.baseColorG = g;
	mat.baseColorB = b;
	mat.metallic = 0.0f;
	mat.roughness = roughness;
	return mat;
}
