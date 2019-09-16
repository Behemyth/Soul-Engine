#pragma once

#include "Compute/ComputeBackend.h"
#include "Core/Utility/Exception/Exception.h"


class CPUBackend : public ComputeBackend<CPUBackend> {

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

template <typename KernelFunction, typename... Args>
void CPUBackend::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {

	throw NotImplemented();

}
