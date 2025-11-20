export module synodic.soul.compute:compute_module;

import :policy;
import std;

export class ComputeModule {
public:
	ComputeModule() = default;
	virtual ~ComputeModule() = default;

	ComputeModule(const ComputeModule&) = delete;
	ComputeModule(ComputeModule&&) noexcept = default;

	ComputeModule& operator=(const ComputeModule&) = delete;
	ComputeModule& operator=(ComputeModule&&) noexcept = default;
};
