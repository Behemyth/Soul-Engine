#pragma once

#include "SparseVector.h"

#include "Types.h"
#include "Core/Structure/SparseStructure.h"
#include "SparseVector.h"
#include "Core/Utility/Exception/Exception.h"

#include <vector>
#include <cassert>


template<class Key, class Value, class Hash = std::hash<Key>>
class SparseHashMap final : public SparseStructure {

	template<bool Const = false>
	class SparseHashMapIterator {

		using KeyIteratorType = typename std::vector<Key>::iterator;
		using ValueIteratorType = typename std::vector<Value>::iterator;

	public:
		
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = Value;
		using difference_type = std::ptrdiff_t;
		using pointer = Value*;
		using reference = Value&;


		SparseHashMapIterator(KeyIteratorType = nullptr, ValueIteratorType = nullptr);

		Value& operator*();
		Value* operator->() const;

	private:
		
		KeyIteratorType keyIterator_;
		ValueIteratorType valueIterator_;

		
	};
	
public:

	using size_type = std::size_t;
	using iterator = SparseHashMapIterator<false>;
	using const_iterator = SparseHashMapIterator<true>;

	SparseHashMap();
	~SparseHashMap() override = default;

	Value& At(const Key&);
	Value& operator[](const Key&);

	Value* Data() noexcept;

	[[nodiscard]] size_type Size() const noexcept;
	
	template<typename... Args>
	std::pair<iterator, bool> Emplace(const Key&, Args&&...);

	void Clear();
	void Erase(const Key& key);

private:

	static constexpr Hash hasher_ = {};
	static constexpr uint8 blockSize_ = 8;

	std::pair<size_t&, bool> Insert(const Key& key, size_t indirection);

	[[nodiscard]] bool Full() const;
	void Rehash(size_t);
	[[nodiscard]] size_t ReduceHash(size_t hash) const;

	float minLoadFactor_;
	float maxLoadFactor_;
	
	SparseVector<size_type, blockSize_> indirectionTable_;
	std::vector<Key> keys_;
	std::vector<Value> values_;


};

template<class Key, class Value, class Hash>
template<bool Const>
SparseHashMap<Key, Value, Hash>::SparseHashMapIterator<Const>::SparseHashMapIterator(
	KeyIteratorType keyIterator,
	ValueIteratorType valueIterator):
	keyIterator_(keyIterator),
	valueIterator_(valueIterator)
{
}

template<class Key, class Value, class Hash>
template<bool Const>
Value& SparseHashMap<Key, Value, Hash>::SparseHashMapIterator<Const>::operator*()
{

	return *valueIterator_;
	
}

template<class Key, class Value, class Hash>
template<bool Const>
Value* SparseHashMap<Key, Value, Hash>::SparseHashMapIterator<Const>::operator->()
	const
{

	return &*valueIterator_;
	
}

template<class Key, class Value, class Hash>
SparseHashMap<Key, Value, Hash>::SparseHashMap() :
	minLoadFactor_(0.20f), 
	maxLoadFactor_(0.80f)
{
}
	

template<class Key, class Value, class Hash>
Value& SparseHashMap<Key, Value, Hash>::At(const Key& key)
{

	const size_t hash = hasher_(key);
	const size_t index = ReduceHash(hash);
	
	auto indirection = indirectionTable_.Find(index);
	return values_.at(*indirection);

}

template<class Key, class Value, class Hash>
Value& SparseHashMap<Key, Value, Hash>::operator[](const Key& key)
{

	//TODO: should not be a part of []
	if (Full()) {

		Rehash(indirectionTable_.Size() * 2);
	}

	auto [indirection, inserted] = Insert(key, values_.size());

	if (inserted) {

		keys_.push_back(key);
		return values_.emplace_back();

	}
	else {

		return values_.at(indirection);

	}

}

template<class Key, class Value, class Hash>
Value* SparseHashMap<Key, Value, Hash>::Data() noexcept
{
	
	return values_.data();
	
}

template<class Key, class Value, class Hash>
typename SparseHashMap<Key, Value, Hash>::size_type SparseHashMap<Key, Value, Hash>::Size() const noexcept
{
	
	return values_.size();
	
}

template<class Key, class Value, class Hash>
template<typename... Args>
std::pair<typename SparseHashMap<Key, Value, Hash>::iterator, bool>
SparseHashMap<Key, Value, Hash>::Emplace(const Key& key, Args&&... args)
{

	if (Full()) {

		Rehash(indirectionTable_.Size() * 2);

	}

	size_t hash = hasher_(key);
 	size_t index = ReduceHash(hash);

	for (size_t i=0; true; ++i) {
		
		auto [itr, inserted] = indirectionTable_.Insert(index, values_.size());
		
		if (inserted) {

			auto& returnKey = keys_.emplace_back(key);
			auto& returnValue = values_.emplace_back(std::forward<Args>(args)...);

			return{iterator(--keys_.end(), --values_.end()), true};

		}

		if (size_type indirection = *itr; hasher_(keys_[indirection]) == hash) {

			return {iterator(keys_.begin() + indirection, values_.begin() + indirection), false};

		}

		//Search for a new spot
		index += ( hash + i * i ) % values_.size();

		//Wrap to the blocksize
		index &= indirectionTable_.BucketCount() * blockSize_ - 1;

	}

	//TODO: should never reach here. Error handling? Dummy return?
	throw NotImplemented();

}

template<class Key, class Value, class Hash>
void SparseHashMap<Key, Value, Hash>::Clear()
{

	indirectionTable_.Clear();
	keys_.clear();
	values_.clear();

}

/*
 * Inserts a hash into the the vector of blocks. A precondition is that there is room for the
 * insertion
 *
 * @param	key	   	The key.
 * @param	indirection	The indirection value pointing to the insert.
 *
 * @returns	A pair;
 */

template<class Key, class Value, class Hash>
std::pair<size_t&, bool> SparseHashMap<Key, Value, Hash>::Insert(const Key& key, size_t indirection) {

	size_t hash = hasher_(key);
	size_t index = ReduceHash(hash);

	for (size_t i=0; true; ++i) {

		auto [iterator, inserted] = indirectionTable_.Insert(index, indirection);

		if (inserted) {

			return {indirection, true};
			
		}

		if (size_t storedIndirection = *iterator; hasher_(keys_[indirection]) == hash) {

			return {storedIndirection, false};
		}

		// Search for a new spot
		index += ( hash + i * i ) % values_.size();

		// Wrap to the blocksize
		index &= indirectionTable_.BucketCount() * blockSize_ - 1;
	}

	//TODO: should never reach here. Error handling? Dummy return?
	throw NotImplemented();

}

template<class Key, class Value, class Hash>
bool SparseHashMap<Key, Value, Hash>::Full() const {

	return values_.size() + 1 > indirectionTable_.BucketCount() * blockSize_ * maxLoadFactor_;

}

template<class Key, class Value, class Hash>
void SparseHashMap<Key, Value, Hash>::Rehash(size_t newBlockSize) {

	indirectionTable_.Clear();
	indirectionTable_.Reserve(std::max(newBlockSize, static_cast<size_t>(4)));

	for (int i = 0; i < keys_.size(); ++i) {

		Insert(keys_[i], i);

	}


}

template<class Key, class Value, class Hash>
void SparseHashMap<Key, Value, Hash>::Erase(const Key& key) {

	//auto indirectionIterator = SparseTable<Key, Hash>::Find(key);

	//if(indirectionIterator != SparseTable<Key, Hash>::end()){

	//	if (values_.size() > 1)
	//	{
	//		//TODO: iterator swap
	//		std::swap(keys_[indirection], *keys.back());
	//		std::swap(values_[indirection], *values_.back());

	//		SparseTable<Key, Hash>::Erase(keys_.back());
	//		keys_.pop_back();
	//		values_.pop_back();

	//		SparseTable<Key, Hash>::Insert(keys_[indirection], indirection);

	//	}
	//	else
	//	{

	//		keys_.clear();
	//		values_.clear();
	//		blocks_.Clear();

	//	}

	//	SparseTable<Key, Hash>::Erase(key);

	//}

	throw NotImplemented();

}

//TODO: this should be a part of a hash library.
template<class Key, class Value, class Hash>
size_t SparseHashMap<Key, Value, Hash>::ReduceHash(size_t hash) const
{

	hash ^= hash >> 7;
	hash *= 11400714819323198485llu >> 7;
	return hash & (indirectionTable_.BucketCount() * blockSize_ - 1);

}