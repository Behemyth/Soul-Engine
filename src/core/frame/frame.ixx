export module synodic.soul.core:frame;

import std;

export class Frame {
public:
	Frame();
	Frame(const Frame& previous, const Frame& host);
	~Frame() = default;

	Frame(const Frame&) = delete;
	Frame(Frame&&) noexcept = default;

	Frame& operator=(const Frame&) = delete;
	Frame& operator=(Frame&&) noexcept = default;

	// Gets
	bool Dirty() const; // flag 0

	// Sets
	void Dirty(bool);

private:
	std::size_t id_;
	std::bitset<1> flags_;
};

// Implementation
inline Frame::Frame() :
	id_(0) {
}

inline Frame::Frame(const Frame& previous, const Frame& host) :
	id_(previous.id_ + 1) {
	Dirty(true);
}

inline bool Frame::Dirty() const {
	return flags_[0];
}

inline void Frame::Dirty(const bool on) {
	flags_[0] = on;
}
