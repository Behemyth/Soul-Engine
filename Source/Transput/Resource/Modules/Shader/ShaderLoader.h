#pragma once

#include "Transput/Resource/ResourceLoader.h"


class ShaderLoader : public ResourceLoader {
	
public:

	ShaderLoader() = default;
	~ShaderLoader() override = default;

	void Load(const std::string_view&) override = 0;


};

