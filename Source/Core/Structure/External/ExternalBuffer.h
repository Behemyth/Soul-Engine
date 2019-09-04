#pragma once

#include "Types.h"
#include "Core/Structure/ExternalStructure.h"

#include <vector>
#include <cassert>
#include <memory>

template<class T>
class ExternalBuffer : public ExternalStructure<T> {

	using size_type = size_t;

public:

	ExternalBuffer() = default;
	~ExternalBuffer() override = default;

	constexpr ExternalBuffer<T>& operator=(const nonstd::span<T> span) noexcept;


};

template<class T>
constexpr ExternalBuffer<T>& ExternalBuffer<T>::operator=(const nonstd::span<T> span) noexcept
{
	data = span;
	return *this;
}
