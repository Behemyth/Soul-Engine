export module synodic.soul.engine:core.material;

import std;

export class Material {
public:
	Material(std::string texName = "Resources//Textures//SoulDefault.png");

	// TODO: Replace with proper vector types (glm removed)
	struct { float r, g, b, a; } diffuse;
	struct { float r, g, b, a; } emit;
	std::string texName_;
};

// Implementation
inline Material::Material(std::string texName) : texName_(std::move(texName)) {
	// Implementation placeholder - was using CUDA texture loading
}
