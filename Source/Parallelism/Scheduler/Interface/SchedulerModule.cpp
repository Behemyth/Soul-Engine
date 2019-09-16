#include "SchedulerModule.h"

#include "Modules/Fiber/FiberSchedulerBackend.h"


SchedulerModule::SchedulerModule(Property<uint>& threadCount): 
	scheduler_(threadCount)
{
}

void SchedulerModule::Block() const
{

	scheduler_.Block();

}

void SchedulerModule::Yield()
{

	scheduler_.Yield();

}

//TODO: This needs to instead be runtime loadable from shared libraries or statically linked
std::shared_ptr<SchedulerModule> SchedulerModule::CreateModule(Property<uint>& threadCount)
{

	return std::make_shared<SchedulerModule>(threadCount);

}
