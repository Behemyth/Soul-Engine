export module synodic.soul.engine:compute.mock.backend;

import :compute.compute_backend;
import synodic.soul.core;

export class MockBackend : public ComputeBackend<MockBackend> {

public:

	MockBackend() = default;
	virtual ~MockBackend() = default;

	MockBackend(const MockBackend&) = delete;
	MockBackend(MockBackend&&) noexcept = default;

	MockBackend& operator=(const MockBackend&) = delete;
	MockBackend& operator=(MockBackend&&) noexcept = default;

	template <typename KernelFunction, typename... Args>
	void Launch(const ComputePolicy&, const KernelFunction&, Args&&...);


};

template <typename KernelFunction, typename... Args>
void MockBackend::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {

	throw NotImplemented();

}
