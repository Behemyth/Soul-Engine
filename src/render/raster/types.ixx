export module synodic.soul.raster:types;

import std;
import synodic.soul.core;

// Temporary type until proper vector types are available
export struct uvec2 { std::uint32_t x, y; };

// Opaque handle for platform/backend-specific surface handles (e.g., VkSurfaceKHR)
// Backends cast to/from their native type using reinterpret_cast
export using NativeSurfaceHandle = std::uint64_t;

export struct ShaderSet
{

	Entity* vertex;

	Entity* tessellationControl;

	Entity* tessellationEvaluation;

	Entity* geometry;

	Entity* fragment;

};

export enum class Format {

	RGBA,
	Unknown

};
