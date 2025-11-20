export module synodic.soul.compute.backend.cuda;

import synodic.soul.compute;
import synodic.soul.core;
import std;

export class CUDABackend : public ComputeModule, public ComputeBackend<CUDABackend> {
public:
	CUDABackend() = default;
	virtual ~CUDABackend() = default;

	CUDABackend(const CUDABackend&) = delete;
	CUDABackend(CUDABackend&&) noexcept = default;

	CUDABackend& operator=(const CUDABackend&) = delete;
	CUDABackend& operator=(CUDABackend&&) noexcept = default;

	template <typename KernelFunction, typename... Args>
	void Launch(const ComputePolicy&, const KernelFunction&, Args&&...);
};

export class CUDADevice : public ComputeDevice<CUDADevice> {
public:
	CUDADevice() = default;
	virtual ~CUDADevice() = default;

	CUDADevice(const CUDADevice&) = delete;
	CUDADevice(CUDADevice&&) noexcept = default;

	CUDADevice& operator=(const CUDADevice&) = delete;
	CUDADevice& operator=(CUDADevice&&) noexcept = default;

	template <typename KernelFunction, typename... Args>
	void Launch(const ComputePolicy&, const KernelFunction&, Args&&...);
};

export template <typename T>
class CUDABuffer : public DeviceBuffer<T> {
public:
	CUDABuffer() = default;
	virtual ~CUDABuffer() = default;

	CUDABuffer(const CUDABuffer&) = delete;
	CUDABuffer(CUDABuffer&&) noexcept = default;

	CUDABuffer& operator=(const CUDABuffer&) = delete;
	CUDABuffer& operator=(CUDABuffer&&) noexcept = default;
};

// Implementations
template <typename KernelFunction, typename... Args>
void CUDABackend::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {
	throw NotImplemented();
}

template <typename KernelFunction, typename... Args>
void CUDADevice::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {
	throw NotImplemented();
}
