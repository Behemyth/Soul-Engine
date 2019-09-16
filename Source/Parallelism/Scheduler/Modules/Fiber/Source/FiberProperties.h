#pragma once

#include "boost/fiber/properties.hpp"
#include "Parallelism/Scheduler/TaskParameters.h"

class FiberProperties : public boost::fibers::fiber_properties {

public:

	//Construction
	FiberProperties(boost::fibers::context*);

	//Implementation
	TaskPriority GetPriority() const;
	int RequiredThread() const;

	void SetProperties(TaskPriority, int);

private:

	TaskPriority priority_;
	int requiredThread_;

};
