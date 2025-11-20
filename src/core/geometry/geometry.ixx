export module synodic.soul.engine:core.geometry;

import std;
import synodic.soul.core;

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

export class GUIVertex : Component {

public:

	GUIVertex() = default;
	~GUIVertex() = default;

	// TODO: Replace with proper vector types (glm removed)
	struct { float x, y; } position;
	struct { float x, y; } textureCoord;
	std::uint32_t colour;

};

export class Face : Component
{

public:

	Face() = default;
	~Face() = default;

	// TODO: Replace with proper vector types (glm removed)
	struct { std::uint32_t x, y, z; } indices;
	std::uint32_t material; //TODO investigate materials
};

export class BoundingBox : Component
{

public:

	BoundingBox() = default;
	~BoundingBox() = default;

	// TODO: Replace with proper vector types (glm removed)
	struct { float x, y, z; } min;
	struct { float x, y, z; } max;

};

export class Tet : Component
{

public:

	Tet() = default;
	~Tet() = default;

	// TODO: Replace with proper vector types (glm removed)
	struct { std::uint32_t x, y, z, w; } indices;
	std::uint32_t material;
	std::uint32_t object;

};
