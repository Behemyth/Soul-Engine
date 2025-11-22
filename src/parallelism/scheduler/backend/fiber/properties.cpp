module synodic.soul.engine.fiber;

Properties::Properties(boost::fibers::context* context) :
	fiber_properties(context),
	priority_(TaskPriority::HIGH),
	requiredThread_(-1)
{
}

TaskPriority Properties::GetPriority() const {
	return priority_;
}

std::int32_t Properties::RequiredThread() const {
	return requiredThread_;
}

void Properties::SetProperties(TaskPriority p, std::int32_t m) {
	if (p != priority_ || m != requiredThread_) {
		priority_ = p;
		requiredThread_ = m;
		notify();
	}
}

