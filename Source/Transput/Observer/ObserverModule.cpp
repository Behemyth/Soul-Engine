#include "ObserverModule.h"

ObserverModule::ObserverModule(std::string_view path): path_(path)
{

}

std::shared_ptr<ObserverModule> ObserverModule::CreateModule()
{
	
	return nullptr;
	
}