export module synodic.soul.engine:compute.opencl.buffer;

import :compute.compute_buffer;

export template <typename T>
class OpenCLBuffer : public DeviceBuffer<T> {

public:

	OpenCLBuffer() = default;
	virtual ~OpenCLBuffer() = default;

	OpenCLBuffer(const OpenCLBuffer&) = delete;
	OpenCLBuffer(OpenCLBuffer&&) noexcept = default;

	OpenCLBuffer& operator=(const OpenCLBuffer&) = delete;
	OpenCLBuffer& operator=(OpenCLBuffer&&) noexcept = default;


};
