export module synodic.soul.window:monitor;

import std;

export class Monitor
{

public:

	Monitor() {}
	virtual ~Monitor() = default;

	Monitor(const Monitor &) = default;
	Monitor(Monitor &&) noexcept = default;

	Monitor& operator=(const Monitor &) = delete;
	Monitor& operator=(Monitor &&) noexcept = default;

	void Scale(float&, float&) const = delete;

	void Position(std::int32_t&, std::int32_t&) const = delete;

	void Size(std::int32_t&, std::int32_t&) const = delete;
	void ColorBits(std::int32_t&, std::int32_t&, std::int32_t&) const = delete;
	void RefreshRate(std::int32_t&) const = delete;

	std::string Name() const = delete;

};
