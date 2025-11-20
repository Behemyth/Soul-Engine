export module synodic.soul.compute:buffer;

export template <typename T>
class ComputeBuffer {
public:
	ComputeBuffer() = default;
	virtual ~ComputeBuffer() = default;

	ComputeBuffer(const ComputeBuffer&) = delete;
	ComputeBuffer(ComputeBuffer&&) noexcept = default;

	ComputeBuffer& operator=(const ComputeBuffer&) = delete;
	ComputeBuffer& operator=(ComputeBuffer&&) noexcept = default;
};

export template <typename T>
class DeviceBuffer {
public:
	DeviceBuffer() = default;
	virtual ~DeviceBuffer() = default;

	DeviceBuffer(const DeviceBuffer&) = delete;
	DeviceBuffer(DeviceBuffer&&) noexcept = default;

	DeviceBuffer& operator=(const DeviceBuffer&) = delete;
	DeviceBuffer& operator=(DeviceBuffer&&) noexcept = default;
};
