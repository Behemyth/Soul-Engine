#pragma once

#include <string>

class Monitor
{

public:

	Monitor() {}
	virtual ~Monitor() = default;

	Monitor(const Monitor &) = default;
	Monitor(Monitor &&) noexcept = default;

	Monitor& operator=(const Monitor &) = delete;
	Monitor& operator=(Monitor &&) noexcept = default;

	void Scale(float&, float&) const = delete;
	
	void Position(int&, int&) const = delete;

	void Size(int&, int&) const = delete;
	void ColorBits(int&, int&, int&) const = delete;
	void RefreshRate(int&) const = delete;

	std::string Name() const = delete;

};
