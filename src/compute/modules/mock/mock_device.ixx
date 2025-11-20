export module synodic.soul.engine:compute.mock.device;

import :compute.compute_device;
import synodic.soul.core;

export class MockDevice : public ComputeDevice<MockDevice> {

public:

	MockDevice() = default;
	virtual ~MockDevice() = default;

	MockDevice(const MockDevice&) = delete;
	MockDevice(MockDevice&&) noexcept = default;

	MockDevice& operator=(const MockDevice&) = delete;
	MockDevice& operator=(MockDevice&&) noexcept = default;


	template <typename KernelFunction, typename... Args>
	void Launch(const ComputePolicy&, const KernelFunction&, Args&&...);


};


template <typename KernelFunction, typename... Args>
void MockDevice::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {

	throw NotImplemented();

}
