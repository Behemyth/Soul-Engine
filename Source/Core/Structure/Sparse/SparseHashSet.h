#pragma once

#include "Types.h"
#include "Core/Structure/SparseStructure.h"
#include "SparseVector.h"
#include "Core/Utility/Exception/Exception.h"

#include <vector>
#include <cassert>


template<class Key, class Hash = std::hash<Key>>
class SparseHashSet final : public SparseStructure {

	using size_type = size_t;

	static constexpr size_type blockSize_ = 8;

public:

	SparseHashSet() = default;
	~SparseHashSet() override = default;


private:

	SparseVector<Key, Hash> indirectionTable_;
	std::vector<Key> keys_;


};