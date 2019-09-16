#pragma once


template <typename T>
class ComputeBuffer {

public:

	ComputeBuffer() = default;
	virtual ~ComputeBuffer() = default;

	ComputeBuffer(const ComputeBuffer&) = delete;
	ComputeBuffer(ComputeBuffer&&) noexcept = default;

	ComputeBuffer& operator=(const ComputeBuffer&) = delete;
	ComputeBuffer& operator=(ComputeBuffer&&) noexcept = default;


};
