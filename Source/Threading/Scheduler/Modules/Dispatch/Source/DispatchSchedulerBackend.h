#pragma once


class DispatchSchedulerBackend {

public:

	~DispatchSchedulerBackend() = default;

	DispatchSchedulerBackend(const DispatchSchedulerBackend&) = delete;
	DispatchSchedulerBackend(DispatchSchedulerBackend&&) noexcept = default;

	DispatchSchedulerBackend& operator=(const DispatchSchedulerBackend&) = delete;
	DispatchSchedulerBackend& operator=(DispatchSchedulerBackend&&) noexcept = default;


};
