#pragma once

#include "Types.h"
#include "Core/Structure/SparseStructure.h"
#include "SparseBitMap.h"
#include "Core/Utility/Exception/Exception.h"

#include <array>
#include <cassert>

template<class T, size_t N, size_t GroupSize>
class SparseArray final : public SparseStructure {

	using size_type = size_t;

	static constexpr size_type blockCount_ = (N + GroupSize - 1) / GroupSize;

public:

	SparseArray() = default;
	~SparseArray() override = default;

	T& operator[](size_type);
	bool Test(size_type) const;

private:

	std::array<SparseBitMap<T, GroupSize>, blockCount_> data_;


};

template<class T, size_t N, size_t GroupSize>
T& SparseArray<T, N, GroupSize>::operator[](size_type index)
{

	throw NotImplemented();

}

template<class T, size_t N, size_t GroupSize>
bool SparseArray<T, N, GroupSize>::Test(size_type index) const
{

	throw NotImplemented();

}