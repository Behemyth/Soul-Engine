#pragma once

#include <thread>

#include "Property.h"
#include "Types.h"

struct SoulParameters {

	SoulParameters() :
		engineRefreshRate(144),
		threadCount(std::thread::hardware_concurrency())
	{

		//may return 0; see https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency
		if (threadCount == 0) {
			threadCount = 1; //guaranteed a single thread
		}

	}

	Property<int> engineRefreshRate; //the update rate tied to physics and...
	Property<uint> threadCount; //the amount of threads at the program's disposal

};
