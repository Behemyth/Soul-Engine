export module synodic.soul.render.graph:parameter;

import synodic.soul.core;
import synodic.soul.raster;
import std;

export struct RenderTaskParameters {
public:
	RenderTaskParameters() = default;
	~RenderTaskParameters() = default;

	std::string name;
	ShaderSet shaders;
};

export struct RenderGraphOutputParameters {
public:
	RenderGraphOutputParameters() = default;
	~RenderGraphOutputParameters() = default;

	std::string name;
	Entity resource;
};

export struct RenderGraphInputParameters {
public:
	RenderGraphInputParameters() = default;
	~RenderGraphInputParameters() = default;

	std::string name;
	Entity resource;
};
