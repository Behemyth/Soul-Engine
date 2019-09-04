#pragma once

#include "Transput/Observer/ObserverModule.h"

class PollObserverModule : public ObserverModule {
	
public:

	PollObserverModule();
	~PollObserverModule() = default;

	Entity CreateObserver(std::string_view) override;
	bool RemoveObserver(Entity) override;

	bool AttachCallback(Entity, std::function<void(ChangeType)>) override;

private:
	
	std::chrono::duration<int, std::milli> delay_;

};
