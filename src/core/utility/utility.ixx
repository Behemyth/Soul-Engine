export module synodic.soul.engine:core.utility;

import std;

export class Timer {
public:
	Timer() : start_(std::chrono::high_resolution_clock::now()) {}

	void Reset() {
		start_ = std::chrono::high_resolution_clock::now();
	}

	// Returns elapsed time in milliseconds
	float Elapsed() const {
		auto now = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_);
		return static_cast<float>(duration.count());
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};
