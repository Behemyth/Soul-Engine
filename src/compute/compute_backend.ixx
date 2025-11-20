export module synodic.soul.compute:backend;

import :policy;

export template<typename T>
class ComputeBackend {
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
inline void ComputeBackend<T>::Launch(const ComputePolicy& policy,
	const KernelFunction& kernel,
	Args&&... parameters) {
	static_cast<T*>(this)->Launch(policy, kernel, std::forward<Args>(parameters)...);
}
