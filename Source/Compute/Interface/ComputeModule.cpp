#include "ComputeModule.h"


std::shared_ptr<ComputeModule> ComputeModule::CreateModule() {

	return std::make_shared<ComputeModule>();

}
