#pragma once

#include "Types.h"

#include <string>

class HashString final {

public:

	//typedefs
	using HashType = uint64;

	//construction
	explicit constexpr HashString(std::string_view);

	//operators
	constexpr operator std::string_view() const;
	constexpr operator HashType() const;

	constexpr bool operator==(const HashString &other) const;
	constexpr bool operator!=(const HashString &other) const;

private:

	constexpr static HashType Hash(HashType, std::string_view);

	//hash constants
	constexpr static HashType offset = 0xcbf29ce484222325;
	constexpr static HashType prime = 0x100000001b3;

	const HashType hash_;
	std::string_view string_;

};

constexpr HashString::HashString(std::string_view string) :
	hash_(Hash(offset, string)),
	string_(string)
{
}

//cast of the data as a readable string
constexpr HashString::operator std::string_view() const {
	return string_;
}

//cast of the data as a numerical value
constexpr HashString::operator HashType() const {
	return hash_;
}

constexpr bool HashString::operator==(const HashString &other) const {
	return hash_ == other.hash_;
}

constexpr bool HashString::operator!=(const HashString &other) const {
	return hash_ != other.hash_;
}

constexpr HashString::HashType HashString::Hash(HashType offset, std::string_view string) {
	return !string.empty() ? offset : Hash((offset^string[0])*prime, string.substr(1));
}

constexpr HashString operator"" _hashed(const char* str, std::size_t) {
	return HashString{ str };
}