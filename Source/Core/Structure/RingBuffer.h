#pragma once

#include <array>

template<typename T, std::size_t Capacity>
class RingBuffer {

public:
	using reference = T&;
	using size_type = std::size_t;
	
	RingBuffer();
	~RingBuffer() = default;

	RingBuffer(const RingBuffer&) = delete;
	RingBuffer(RingBuffer&&) noexcept = default;

	RingBuffer& operator=(const RingBuffer&) = delete;
	RingBuffer& operator=(RingBuffer&&) noexcept = default;

	bool operator==(const RingBuffer& other);
	bool operator==(RingBuffer& other);

	[[nodiscard]] const T& Back() const;

	const T& Front();

	reference operator[](std::size_t i);

	void Push(const T&);
	void Push(T&&);

	[[nodiscard]] size_type Size() const;

	
private:

	size_type front_;
	size_type size_;

	std::array<T, Capacity> data_;

	void Push_();
	
};

template<typename T, std::size_t Capacity>
RingBuffer<T, Capacity>::RingBuffer(): front_(0), size_(Capacity), data_()
{
}


template<typename T, std::size_t Capacity>
void RingBuffer<T, Capacity>::Push(const T& value)
{

	Push_();
	data_[front_] = value;
}


template<typename T, std::size_t Capacity>
inline bool RingBuffer<T, Capacity>::operator==(const RingBuffer& other)
{
	if (this->size_ != other.size_) {
		return false;
	}
	for (int i = 0; i < this->size_; ++i) {
		if (this->data_[i] != other.data_[i])
			return false;
	}
	return true;
}

template<typename T, std::size_t Capacity>
inline bool RingBuffer<T, Capacity>::operator==(RingBuffer& other)
{
	if (this->size_ != other.size_) {
		return false;
	}
	for (int i = 0; i < this->size_; ++i) {
		if (this->data_[i] != other.data_[i])
			return false;
	}
	return true;
}

template<typename T, std::size_t Capacity>
inline const T& RingBuffer<T, Capacity>::Back() const
{
	if (this->size_ == 0)
		return NULL;
	return this->data_[this->size_ - 1];
}

template<typename T, std::size_t Capacity>
inline const T& RingBuffer<T, Capacity>::Front()
{
	if (this->size_ == 0)
		return NULL;
	return this->data_[0];
}


template<typename T, std::size_t Capacity>
void RingBuffer<T, Capacity>::Push(T&& value)
{

	Push_();
	data_[front_] = std::move(value);
}

template<typename T, std::size_t Capacity>
typename RingBuffer<T, Capacity>::reference RingBuffer<T, Capacity>::operator[](const std::size_t i)
{
	const auto index = (i + front_) % size_;
	return data_[index];
}


template<typename T, std::size_t Capacity>
void RingBuffer<T, Capacity>::Push_()
{

	front_ = (front_ + size_ - 1) % size_;
}

template<typename T, std::size_t Capacity>
typename RingBuffer<T, Capacity>::size_type RingBuffer<T, Capacity>::Size() const
{
	return size_;
}