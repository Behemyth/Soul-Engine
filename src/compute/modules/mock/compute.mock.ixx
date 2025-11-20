export module synodic.soul.compute.backend.mock;

import synodic.soul.compute;
import synodic.soul.core;
import std;

export class MockBackend : public ComputeModule, public ComputeBackend<MockBackend> {
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

export template <typename T>
class MockBuffer : public DeviceBuffer<T> {
public:
	MockBuffer() = default;
	virtual ~MockBuffer() = default;

	MockBuffer(const MockBuffer&) = delete;
	MockBuffer(MockBuffer&&) noexcept = default;

	MockBuffer& operator=(const MockBuffer&) = delete;
	MockBuffer& operator=(MockBuffer&&) noexcept = default;
};

// Implementations
template <typename KernelFunction, typename... Args>
void MockBackend::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {
	throw NotImplemented();
}

template <typename KernelFunction, typename... Args>
void MockDevice::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {
	throw NotImplemented();
}
