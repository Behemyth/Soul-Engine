export module synodic.soul.compute.backend.cpu;

import synodic.soul.compute;
import synodic.soul.core;
import std;

export class CPUBackend : public ComputeModule, public ComputeBackend<CPUBackend> {
public:
	CPUBackend() = default;
	virtual ~CPUBackend() = default;

	CPUBackend(const CPUBackend&) = delete;
	CPUBackend(CPUBackend&&) noexcept = default;

	CPUBackend& operator=(const CPUBackend&) = delete;
	CPUBackend& operator=(CPUBackend&&) noexcept = default;

	template <typename KernelFunction, typename... Args>
	void Launch(const ComputePolicy&, const KernelFunction&, Args&&...);
};

export class CPUDevice : public ComputeDevice<CPUDevice> {
public:
	CPUDevice() = default;
	virtual ~CPUDevice() = default;

	CPUDevice(const CPUDevice&) = delete;
	CPUDevice(CPUDevice&&) noexcept = default;

	CPUDevice& operator=(const CPUDevice&) = delete;
	CPUDevice& operator=(CPUDevice&&) noexcept = default;

	template <typename KernelFunction, typename... Args>
	void Launch(const ComputePolicy&, const KernelFunction&, Args&&...);
};

export template <typename T>
class CPUBuffer : public DeviceBuffer<T> {
public:
	CPUBuffer() = default;
	virtual ~CPUBuffer() = default;

	CPUBuffer(const CPUBuffer&) = delete;
	CPUBuffer(CPUBuffer&&) noexcept = default;

	CPUBuffer& operator=(const CPUBuffer&) = delete;
	CPUBuffer& operator=(CPUBuffer&&) noexcept = default;
};

// Implementations
template <typename KernelFunction, typename... Args>
void CPUBackend::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {
	throw std::runtime_error("CPUBackend::Launch not yet implemented");
}

template <typename KernelFunction, typename... Args>
void CPUDevice::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {
	throw std::runtime_error("CPUDevice::Launch not yet implemented");
}
