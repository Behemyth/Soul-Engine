module;

#include <boost/fiber/properties.hpp>

export module synodic.soul.engine.fiber:properties;

import synodic.soul.engine;

export class Properties : public boost::fibers::fiber_properties {

public:

	//Construction
	Properties(boost::fibers::context*);

	//Implementation
	TaskPriority GetPriority() const;
	std::int32_t RequiredThread() const;

	void SetProperties(TaskPriority, std::int32_t);

private:

	TaskPriority priority_;
	std::int32_t requiredThread_;

};
