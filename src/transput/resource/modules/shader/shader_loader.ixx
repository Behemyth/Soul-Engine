export module synodic.soul.transput:shader_loader;

import :resource;
import std;

export class ShaderLoader : public ResourceLoader {

public:

	ShaderLoader() = default;
	~ShaderLoader() override = default;

	void Load(const std::string_view&) override = 0;


};
