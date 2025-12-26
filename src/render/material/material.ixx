export module synodic.soul.render.material;

import std;
import synodic.periapsis;
import synodic.soul.core;

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
	peri::math::mat4 model;          // 64 bytes - object to world
	peri::math::mat4 viewProjection; // 64 bytes - world to clip
	peri::math::mat4 normalMatrix;   // 64 bytes - for transforming normals (inverse transpose of model)
};

static_assert(sizeof(TransformData) == 192, "TransformData must be 192 bytes");

// Compact push constant data (fits in 128 bytes)
export struct PushConstantData {
	peri::math::mat4 mvp;           // 64 bytes - model-view-projection
	peri::math::mat4 model;         // 64 bytes - model matrix for lighting
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
