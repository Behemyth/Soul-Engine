#pragma once


template <typename T>
class DeviceBuffer {

public:

	DeviceBuffer() = default;
	virtual ~DeviceBuffer() = default;

	DeviceBuffer(const DeviceBuffer&) = delete;
	DeviceBuffer(DeviceBuffer&&) noexcept = default;

	DeviceBuffer& operator=(const DeviceBuffer&) = delete;
	DeviceBuffer& operator=(DeviceBuffer&&) noexcept = default;


};
