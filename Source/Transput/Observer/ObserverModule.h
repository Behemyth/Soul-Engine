#pragma once

#include "Core/Interface/Module/Module.h"

#include <filesystem>

class ObserverModule : Module<ObserverModule> {
	
public:

	ObserverModule(std::string_view);
	~ObserverModule() = default;

	// Factory
	static std::shared_ptr<ObserverModule> CreateModule();
	
private:

    std::filesystem::path path_;


};
