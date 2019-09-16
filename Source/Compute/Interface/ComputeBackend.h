#pragma once

#include "Core/Utility/Template/CRTP.h"
#include "ComputePolicy.h"


template<typename T>
class ComputeBackend : public CRTP<T ,ComputeBackend> {

public:

	ComputeBackend() = default;
	virtual ~ComputeBackend() = default;

	ComputeBackend(const ComputeBackend&) = delete;
	ComputeBackend(ComputeBackend&&) noexcept = default;

	ComputeBackend& operator=(const ComputeBackend&) = delete;
	ComputeBackend& operator=(ComputeBackend&&) noexcept = default;


    template <typename KernelFunction, typename... Args>
	void Launch(const ComputePolicy&, const KernelFunction&, Args&&...);

};

template <typename T>
template <typename KernelFunction, typename... Args>
void ComputeBackend<T>::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {

	this->Type()->Launch(policy, kernel, std::forward<Args>(parameters)...);

}
