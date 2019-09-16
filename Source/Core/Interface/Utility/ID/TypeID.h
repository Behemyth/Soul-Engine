#pragma once

#include "Types.h"

#include <atomic>

template<typename T>
class TypeID
{

public:

	TypeID();


protected:

	uint id_;

private:

	static std::atomic<uint> counter_;

};


template<typename T>
TypeID<T>::TypeID():
	id_(++counter_)
{
}


template<typename T>
std::atomic<uint> TypeID<T>::counter_ = 0;