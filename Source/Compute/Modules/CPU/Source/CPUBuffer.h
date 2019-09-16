#pragma once

#include "Compute/DeviceBuffer.h"


template <typename T>
class CPUBuffer : public DeviceBuffer<T> {

public:

	CPUBuffer() = default;
	virtual ~CPUBuffer() = default;

	CPUBuffer(const CPUBuffer&) = delete;
	CPUBuffer(CPUBuffer&&) noexcept = default;

	CPUBuffer& operator=(const CPUBuffer&) = delete;
	CPUBuffer& operator=(CPUBuffer&&) noexcept = default;


};
