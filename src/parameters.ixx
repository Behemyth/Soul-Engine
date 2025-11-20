export module synodic.soul.core:parameters;

import std;

import :types;

namespace synodic::soul
{
	export struct Parameters
	{
		Parameters() :

			engineRefreshRate(144),

			threadCount(std::thread::hardware_concurrency())

		{
			// may return 0; see https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency

			if (threadCount == 0)
			{
				threadCount = 1;  // guaranteed a single thread
			}
		}

		Property<std::int32_t> engineRefreshRate;  // TODO: Move to display/raster
		Property<std::uint32_t> threadCount;  // TODO: Move to scheduler
	};
}

export enum class WindowType
{
	WINDOWED,
	FULLSCREEN,
	BORDERLESS,
	EMPTY
};

export class WindowParameters
{
public:
	WindowType type;

	std::string title;	// TODO: Replace with proper vector types (glm removed)

	// TODO: Replace with proper vector types (glm removed)	std::array<std::uint32_t, 2> pixelPosition;

	// Was: glm::uvec2 pixelPosition;	std::array<std::uint32_t, 2> pixelSize;

	// Was: glm::uvec2 pixelSize;	std::int32_t monitor;

	struct
	{
		std::uint32_t x, y;
	} pixelPosition;

	struct
	{
		std::uint32_t x, y;
	} pixelSize;

	std::int32_t monitor;
};
