export module synodic.soul.engine:core.geometry;

import std;
import synodic.periapsis;
import synodic.soul.core;

export class Vertex : Component
{

public:

	Vertex() = default;
	~Vertex() = default;

	peri::math::vec3 position;
	peri::math::vec3 normal;
	peri::math::vec2 textureCoord;
	peri::math::vec3 velocity;

	std::uint32_t object;

};

export class GUIVertex : Component {

public:

	GUIVertex() = default;
	~GUIVertex() = default;

	peri::math::vec2 position;
	peri::math::vec2 textureCoord;
	std::uint32_t colour;

};

export class Face : Component
{

public:

	Face() = default;
	~Face() = default;

	peri::math::uvec3 indices;
	std::uint32_t material; //TODO investigate materials
};

export class BoundingBox : Component
{

public:

	BoundingBox() = default;
	~BoundingBox() = default;

	peri::math::vec3 min;
	peri::math::vec3 max;

};

export class Tet : Component
{

public:

	Tet() = default;
	~Tet() = default;

	peri::math::uvec4 indices;
	std::uint32_t material;
	std::uint32_t object;

};
