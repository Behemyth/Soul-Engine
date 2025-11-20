export module synodic.soul.engine:compute.cpu.buffer;

import :compute.buffer;

export template <typename T>
class CPUBuffer : public DeviceBuffer<T> {

public:

	CPUBuffer() = default;
	virtual ~CPUBuffer() = default;

	CPUBuffer(const CPUBuffer&) = delete;
	CPUBuffer(CPUBuffer&&) noexcept = default;

	CPUBuffer& operator=(const CPUBuffer&) = delete;
	CPUBuffer& operator=(CPUBuffer&&) noexcept = default;


};
