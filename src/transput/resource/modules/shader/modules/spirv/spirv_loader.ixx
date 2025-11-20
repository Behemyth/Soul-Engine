export module synodic.soul.transput:spirv_loader;

import :shader_loader;
import std;

export class SPIRVLoader : public ShaderLoader {

public:

	SPIRVLoader() = default;
	~SPIRVLoader() override = default;

	void Load(const std::string_view&) override;


};
