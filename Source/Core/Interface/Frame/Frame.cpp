#include "Frame.h"

Frame::Frame() :
	id_(0) 
{
	
}

Frame::Frame(const Frame& previous, const Frame& host) :
	id_(previous.id_ + 1)
{

	Dirty(true);

}

bool Frame::Dirty() const {
	return flags_[0];
}

void Frame::Dirty(const bool on) {
	flags_[0] = on;
}