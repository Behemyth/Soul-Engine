#pragma once

#include "Compute/ComputeBackend.h"
#include "Core/Utility/Exception/Exception.h"


class CUDABackend : public ComputeBackend<CUDABackend> {

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

template <typename KernelFunction, typename... Args>
void CUDABackend::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {

	throw NotImplemented();

}
