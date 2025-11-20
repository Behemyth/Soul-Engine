export module synodic.soul.compute:policy;

import std;
import synodic.soul.core;

export class ComputePolicy {
public:
	// Constructors + Destructors
	ComputePolicy() = default;
	ComputePolicy(std::uint32_t, std::uint32_t, std::int32_t, std::int32_t);

	// Policy Helpers
	std::uint32_t GetThreadCount() const;

	// TODO: Replace with proper vector types (glm removed)
	struct { std::uint32_t x, y, z; } gridsize;
	struct { std::uint32_t x, y, z; } blocksize;
	std::int32_t sharedMemory;
	std::int32_t stream;
};

inline ComputePolicy::ComputePolicy(std::uint32_t size_, std::uint32_t blockSize_, std::int32_t sharedBytes, std::int32_t stream_) :
	gridsize{(size_ + blockSize_ - 1) / blockSize_, 1, 1},
	blocksize{blockSize_, 1, 1},
	sharedMemory(sharedBytes),
	stream(stream_) {
}

inline std::uint32_t ComputePolicy::GetThreadCount() const {
	return (gridsize.x * gridsize.y * gridsize.z) * (blocksize.x * blocksize.y * blocksize.z);
}
