#pragma once

#include "ComputePolicy.h"
#include "Core/Utility/Template/CRTP.h"


template<typename T>
class ComputeDevice : public CRTP<T, ComputeDevice> {

public:

    ComputeDevice() = default;
	virtual ~ComputeDevice() = default;

	ComputeDevice(const ComputeDevice&) = delete;
	ComputeDevice(ComputeDevice&&) noexcept = default;

	ComputeDevice& operator=(const ComputeDevice&) = delete;
	ComputeDevice& operator=(ComputeDevice&&) noexcept = default;

    template <typename KernelFunction, typename... Args>
	void Launch(const ComputePolicy&, const KernelFunction&, Args&&...);


};

template <typename T>
template <typename KernelFunction, typename... Args>
void ComputeDevice<T>::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {

	this->Type()->Launch(policy, kernel, std::forward<Args>(parameters)...);

}
