#pragma once

#include "Compute/DeviceBuffer.h"


template <typename T>
class CUDABuffer : public DeviceBuffer<T> {

public:

	CUDABuffer() = default;
	virtual ~CUDABuffer() = default;

	CUDABuffer(const CUDABuffer&) = delete;
	CUDABuffer(CUDABuffer&&) noexcept = default;

	CUDABuffer& operator=(const CUDABuffer&) = delete;
	CUDABuffer& operator=(CUDABuffer&&) noexcept = default;


};
