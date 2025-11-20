export module synodic.soul.engine:core.scene;

import std;

// Most of Scene.h is commented out, providing minimal interface
export class Scene {
	// Implementation deferred - requires BVH and compute buffer infrastructure
};

export class Sky {
public:
	Sky() = default;
	Sky(std::string texName);

	void UpdateSky();

private:
	// Image* image; - deferred until texture system is ready
	std::string texName_;
};

// Implementation
inline Sky::Sky(std::string texName) : texName_(std::move(texName)) {
	// Implementation placeholder - was using CUDA texture loading
}

void Sky::UpdateSky() {
	// Implementation placeholder
}
