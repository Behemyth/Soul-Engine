#pragma once

#include "Core/Interface/Module/Module.h"
#include "Core/Composition/Entity/Entity.h"

#include <filesystem>
#include <functional>

enum class ChangeType{
	Creation
};

class ObserverModule : Module<ObserverModule> {
	
public:

	ObserverModule() = default;
	~ObserverModule() = default;

	virtual Entity CreateObserver(std::string_view) = 0;
	virtual bool RemoveObserver(Entity) = 0;

	virtual bool AttachCallback(Entity, std::function<void(ChangeType)>) = 0;
	
	// Factory
	static std::shared_ptr<ObserverModule> CreateModule();
	
private:

    std::filesystem::path path_;


};
