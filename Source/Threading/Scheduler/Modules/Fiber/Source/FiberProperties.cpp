#include "FiberProperties.h"

FiberProperties::FiberProperties(boost::fibers::context* context) :
	fiber_properties(context),
	priority_(TaskPriority::HIGH),
	requiredThread_(-1)
{
}

TaskPriority FiberProperties::GetPriority() const {
	return priority_;
}

int FiberProperties::RequiredThread() const {
	return requiredThread_;
}

void FiberProperties::SetProperties(TaskPriority p, int m) {
	if (p != priority_ || m != requiredThread_) {
		priority_ = p;
		requiredThread_ = m;
		notify();
	}
}

