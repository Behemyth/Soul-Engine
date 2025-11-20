module synodic.soul.engine.fiber;

FiberProperties::FiberProperties(boost::fibers::context* context) :
	fiber_properties(context),
	priority_(TaskPriority::HIGH),
	requiredThread_(-1)
{
}

TaskPriority FiberProperties::GetPriority() const {
	return priority_;
}

std::int32_t FiberProperties::RequiredThread() const {
	return requiredThread_;
}

void FiberProperties::SetProperties(TaskPriority p, std::int32_t m) {
	if (p != priority_ || m != requiredThread_) {
		priority_ = p;
		requiredThread_ = m;
		notify();
	}
}

