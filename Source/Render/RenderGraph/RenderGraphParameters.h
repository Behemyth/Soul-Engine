#pragma once

#include "Composition/Entity/Interface/Entity.h"
#include "Render/Raster/RenderTypes.h"

#include <string>

struct RenderTaskParameters {

public:
	RenderTaskParameters() = default;
	~RenderTaskParameters() = default;

	std::string name;

	ShaderSet shaders;
	
};

struct RenderGraphOutputParameters {

public:
	RenderGraphOutputParameters() = default;
	~RenderGraphOutputParameters() = default;

	std::string name;
	Entity resource;

};

struct RenderGraphInputParameters {

public:
	RenderGraphInputParameters() = default;
	~RenderGraphInputParameters() = default;

	std::string name;
	Entity resource;

};
