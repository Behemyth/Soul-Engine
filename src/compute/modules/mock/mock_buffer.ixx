export module synodic.soul.engine:compute.mock.buffer;

import :compute.compute_buffer;

export template <typename T>
class MockBuffer : public DeviceBuffer<T> {

public:

	MockBuffer() = default;
	virtual ~MockBuffer() = default;

	MockBuffer(const MockBuffer&) = delete;
	MockBuffer(MockBuffer&&) noexcept = default;

	MockBuffer& operator=(const MockBuffer&) = delete;
	MockBuffer& operator=(MockBuffer&&) noexcept = default;


};
