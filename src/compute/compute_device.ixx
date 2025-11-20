export module synodic.soul.compute:device;

import :policy;

export template<typename T>
class ComputeDevice {
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
inline void ComputeDevice<T>::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {
	static_cast<T*>(this)->Launch(policy, kernel, std::forward<Args>(parameters)...);
}
