export module synodic.soul.engine:core.geometry;

import std;
import synodic.library;
import synodic.soul.core;

export class Vertex : Component
{

public:

	Vertex() = default;
	~Vertex() = default;

	synodic::math::vec3 position;
	synodic::math::vec3 normal;
	synodic::math::vec2 textureCoord;
	synodic::math::vec3 velocity;

	std::uint32_t object;

};

export class GUIVertex : Component {

public:

	GUIVertex() = default;
	~GUIVertex() = default;

	synodic::math::vec2 position;
	synodic::math::vec2 textureCoord;
	std::uint32_t colour;

};

export class Face : Component
{

public:

	Face() = default;
	~Face() = default;

	synodic::math::uvec3 indices;
	std::uint32_t material; //TODO investigate materials
};

export class BoundingBox : Component
{

public:

	BoundingBox() = default;
	~BoundingBox() = default;

	synodic::math::vec3 min;
	synodic::math::vec3 max;

};

export class Tet : Component
{

public:

	Tet() = default;
	~Tet() = default;

	synodic::math::uvec4 indices;
	std::uint32_t material;
	std::uint32_t object;

};
