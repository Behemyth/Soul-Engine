#pragma once

#include "Transput/Resource/Modules/Shader/ShaderLoader.h"

class SPIRVLoader : public ShaderLoader {
	
public:

	SPIRVLoader() = default;
	~SPIRVLoader() override = default;

	void Load(const std::string_view&) override;


};

