#include "PollObserverModule.h"

#include "Core/Utility/Exception/Exception.h"

PollObserverModule::PollObserverModule():
	delay_(1000)
{

	throw NotImplemented();
	
}

Entity PollObserverModule::CreateObserver(std::string_view)
{
}

bool PollObserverModule::RemoveObserver(Entity)
{

	return false;
}

bool PollObserverModule::AttachCallback(Entity, std::function<void(ChangeType)>)
{

	return false;
}