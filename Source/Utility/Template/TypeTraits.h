#pragma once

#include <type_traits>

//std extensions, so follows same style
template<class T> 
struct dependent_false : std::false_type {};

template <class T>
constexpr auto dependent_false_v = dependent_false<T>::value;