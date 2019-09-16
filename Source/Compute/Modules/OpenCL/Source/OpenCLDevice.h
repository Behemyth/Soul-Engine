#pragma once

#include "Compute/ComputeDevice.h"
#include "Core/Utility/Exception/Exception.h"


class OpenCLDevice : public ComputeDevice<OpenCLDevice> {

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


template <typename KernelFunction, typename... Args>
void OpenCLDevice::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {

	throw NotImplemented();

}