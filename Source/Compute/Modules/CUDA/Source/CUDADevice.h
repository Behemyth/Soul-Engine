#pragma once

#include "Compute/ComputeDevice.h"
#include "Core/Utility/Exception/Exception.h"


class CUDADevice : public ComputeDevice<CUDADevice> {

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


template <typename KernelFunction, typename... Args>
void CUDADevice::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {

	throw NotImplemented();

}