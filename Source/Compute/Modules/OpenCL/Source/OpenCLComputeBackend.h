#pragma once

#include "Compute/ComputeBackend.h"
#include "Core/Utility/Exception/Exception.h"

class OpenCLBackend : public ComputeBackend<OpenCLBackend> {

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

template <typename KernelFunction, typename... Args>
void OpenCLBackend::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {

	throw NotImplemented();

}
