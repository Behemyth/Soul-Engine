export module synodic.soul.compute.backend.opencl;

import synodic.soul.compute;
import synodic.soul.core;
import std;

export class OpenCLBackend : public ComputeModule, public ComputeBackend<OpenCLBackend> {
public:
	OpenCLBackend() = default;
	virtual ~OpenCLBackend() = default;

	OpenCLBackend(const OpenCLBackend&) = delete;
	OpenCLBackend(OpenCLBackend&&) noexcept = default;

	OpenCLBackend& operator=(const OpenCLBackend&) = delete;
	OpenCLBackend& operator=(OpenCLBackend&&) noexcept = default;

	template <typename KernelFunction, typename... Args>
	void Launch(const ComputePolicy&, const KernelFunction&, Args&&...);
};

export class OpenCLDevice : public ComputeDevice<OpenCLDevice> {
public:
	OpenCLDevice() = default;
	virtual ~OpenCLDevice() = default;

	OpenCLDevice(const OpenCLDevice&) = delete;
	OpenCLDevice(OpenCLDevice&&) noexcept = default;

	OpenCLDevice& operator=(const OpenCLDevice&) = delete;
	OpenCLDevice& operator=(OpenCLDevice&&) noexcept = default;

	template <typename KernelFunction, typename... Args>
	void Launch(const ComputePolicy&, const KernelFunction&, Args&&...);
};

export template <typename T>
class OpenCLBuffer : public DeviceBuffer<T> {
public:
	OpenCLBuffer() = default;
	virtual ~OpenCLBuffer() = default;

	OpenCLBuffer(const OpenCLBuffer&) = delete;
	OpenCLBuffer(OpenCLBuffer&&) noexcept = default;

	OpenCLBuffer& operator=(const OpenCLBuffer&) = delete;
	OpenCLBuffer& operator=(OpenCLBuffer&&) noexcept = default;
};

// Implementations
template <typename KernelFunction, typename... Args>
void OpenCLBackend::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {
	throw NotImplemented();
}

template <typename KernelFunction, typename... Args>
void OpenCLDevice::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {
	throw NotImplemented();
}
