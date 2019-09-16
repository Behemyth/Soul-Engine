#pragma once

#include <cstddef>
#include <bitset>

class Frame {

public:

	Frame();
	Frame(const Frame&, const Frame&);
	~Frame() = default;

	Frame(const Frame&) = delete;
	Frame(Frame&&) noexcept = default;

	Frame& operator=(const Frame&) = delete;
	Frame& operator=(Frame&&) noexcept = default;

	//Gets
	bool Dirty() const; //flag 0

	//Sets
	void Dirty(bool);

private:

	std::size_t id_;

	std::bitset<1> flags_;

};
