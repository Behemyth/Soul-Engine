#pragma once

#include "Types.h"

#include <string>

enum class WindowType { WINDOWED, FULLSCREEN, BORDERLESS, EMPTY };

class WindowParameters {

public:

	WindowType type;
	std::string title;
	glm::uvec2 pixelPosition;
	glm::uvec2 pixelSize;
	int monitor;

};