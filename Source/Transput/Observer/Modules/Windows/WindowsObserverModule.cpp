#include "WindowsObserverModule.h"
#include "Core/Utility/Exception/Exception.h"

#include <windows.h>

WindowsObserverModule::WindowsObserverModule()
{

	throw NotImplemented();
	
}

Entity WindowsObserverModule::CreateObserver(std::string_view)
{

	
}

bool WindowsObserverModule::RemoveObserver(Entity)
{

	return false;
	
}

bool WindowsObserverModule::AttachCallback(Entity, std::function<void(ChangeType)>)
{
	
	return false;
	
}