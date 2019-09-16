#pragma once

#include "Types.h"
#include "Core/Structure/SparseStructure.h"
#include "Core/Utility/Exception/Exception.h"

#include <vector>
#include <cassert>
#include <memory>
#include <type_traits>
#include <bitset>

template<class T, uint8 N>
class SparseBitMap final : public SparseStructure {
	
	static_assert(N % 4 == 0, "N must be byte aligned.");


public:

	using size_type = uint8;
	using iterator = typename std::vector<T>::iterator;


	SparseBitMap() = default;
	~SparseBitMap() = default;

	T& operator[](size_type);
	[[nodiscard]] bool Test(size_type) const;

	template<typename... Args>
	std::pair<iterator, bool> Emplace(size_type, Args&&...);
	std::pair<iterator, bool> Insert(size_type position, const T& value);

	void Clear();

	iterator Find(const size_type position);
	
	iterator begin() noexcept;
	iterator end() noexcept;
	
private:

	[[nodiscard]] uint8 CountPosition(const size_type position) const;
	
	std::bitset<N> mapping_;
	std::vector<T> data_;


};

template<class T, uint8 N>
T& SparseBitMap<T, N>::operator[](size_type position)
{

	assert(mapping_.test(position));

	uint8 pos = CountPosition(position) - 1;

	assert(pos < data_.size());

	return data_[pos];

}

template<class T, uint8 N>
bool SparseBitMap<T, N>::Test(size_type position) const
{

	return mapping_.test(position);

}

template<class T, uint8 N>
template<typename... Args>
std::pair<typename SparseBitMap<T, N>::iterator, bool> SparseBitMap<T, N>::Emplace(size_type position,
	Args&&... args)
{

	if (Test(position)) {
		return {operator[](position), false};
	}

	uint8 pos = CountPosition(position);
	mapping_.set(position);
	
	auto iter = data_.emplace(data_.begin() + pos, std::forward<Args>(args)...);

	return {iter, true};
}

/*
 * Inserts
 *
 * @param	position	The hash.
 * @param	value	The indirection.
 *
 * @returns	A pair;
 */
template<class T, uint8 N>
std::pair<typename SparseBitMap<T, N>::iterator, bool> SparseBitMap<T, N>::Insert(
	size_type position,
	const T& value)
{

	if (Test(position)) {
		return {data_.begin() + position, false};
	}

	uint8 pos = CountPosition(position);
	mapping_.set(position);
	
	auto iter = data_.insert(data_.begin() + pos, value);

	return {iter, true};

}

template<class T, uint8 N>
void SparseBitMap<T, N>::Clear()
{

	mapping_.reset();
	data_.resize(0);

}

template<class T, uint8 N>
typename SparseBitMap<T, N>::iterator SparseBitMap<T, N>::Find(const size_type position)
{

	if (!Test(position)) {
		return data_.end();
	}

	uint8 pos = CountPosition(position);
	
	return data_.begin() + pos;
	
}

template<class T, uint8 N>
typename SparseBitMap<T, N>::iterator SparseBitMap<T, N>::begin() noexcept
{

	return data_.begin();
	
}

template<class T, uint8 N>
typename SparseBitMap<T, N>::iterator SparseBitMap<T, N>::end() noexcept
{

	return data_.end();

}

template<class T, uint8 N>
uint8 SparseBitMap<T, N>::CountPosition(const size_type position) const
{
	
	std::bitset<N> mask;
	mask = ~(~mask << position) & mapping_;

	// TODO: Replace with `popcount`
	return static_cast<uint8>(mask.count());
	
}