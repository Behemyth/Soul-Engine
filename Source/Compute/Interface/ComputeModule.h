#pragma once

#include "Core/Interface/Module/Module.h"
#include "ComputePolicy.h"

#include <memory>
#include <vector>

#include "Modules/CPU/CPUBackend.h"
#include "Modules/CUDA/CUDABackend.h"
#include "Modules/OpenCL/OpenCLBackend.h"

#include "Modules/CPU/CPUDevice.h"
#include "Modules/CUDA/CUDADevice.h"
#include "Modules/OpenCL/OpenCLDevice.h"


class ComputeModule : public Module<ComputeModule> {

public:

	ComputeModule() = default;
	virtual ~ComputeModule() = default;

	ComputeModule(const ComputeModule&) = delete;
	ComputeModule(ComputeModule&&) noexcept = default;

	ComputeModule& operator=(const ComputeModule&) = delete;
	ComputeModule& operator=(ComputeModule&&) noexcept = default;


    template <typename KernelFunction, typename... Args>
	void Launch(const ComputePolicy& policy,
		const KernelFunction& kernel,
		Args&&... parameters) {
        
    }


    //Factory
	static std::shared_ptr<ComputeModule> CreateModule();


private:

    CUDABackend CUDABackend_;
	OpenCLBackend OpenCLBackend_;
	CPUBackend CPUBackend_;

    std::vector<CUDADevice> CUDADevices_;
	std::vector<OpenCLDevice> OpenCLDevices_;
	CPUDevice CPUDevice_;

};
