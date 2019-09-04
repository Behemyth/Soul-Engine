#pragma once

#include "Transput/Observer/ObserverModule.h"

class WindowsObserverModule : public ObserverModule {
	
public:

	WindowsObserverModule();
	~WindowsObserverModule() = default;

	Entity CreateObserver(std::string_view) override;
	bool RemoveObserver(Entity) override;

	bool AttachCallback(Entity, std::function<void(ChangeType)>) override;

private:


};
