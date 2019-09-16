#pragma once

#include <memory>

class RasterDevice {

public:

	RasterDevice() = default;
	virtual ~RasterDevice() = default;

	RasterDevice(const RasterDevice &) = delete;
	RasterDevice(RasterDevice &&) noexcept = default;

	RasterDevice& operator=(const RasterDevice &) = delete;
	RasterDevice& operator=(RasterDevice &&) noexcept = default;

	virtual void Synchronize() = 0;

};
