export module synodic.soul.engine:compute.cpu.device;

import :compute.device;
import synodic.soul.core;

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


template <typename KernelFunction, typename... Args>
void CPUDevice::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {

	throw std::runtime_error("CPUDevice::Launch not yet implemented");

}
