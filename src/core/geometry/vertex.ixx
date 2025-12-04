export module synodic.soul.engine:core.geometry.vertex;

import std;
import synodic.soul.core;

// TODO: Replace with proper vector types (glm removed)


export class Vertex : Component
{

public:

	Vertex() = default;
	~Vertex() = default;

	// TODO: Replace with proper vector types (glm removed)
	struct { float x, y, z; } position;
	struct { float x, y, z; } normal;
	struct { float x, y; } textureCoord;
	struct { float x, y, z; } velocity;

	std::uint32_t object;

};

class GUIVertex : Component {

public:

	GUIVertex() = default;
	~GUIVertex() = default;

	// TODO: Replace with proper vector types (glm removed)
	struct { float x, y; } position;
	struct { float x, y; } textureCoord;
	std::uint32_t colour;

};
