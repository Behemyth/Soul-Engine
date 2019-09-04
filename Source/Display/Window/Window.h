#pragma once

#include "WindowParameters.h"
#include "Core/Composition/Entity/Entity.h"


class Window
{

public:

	Window(const WindowParameters&);
	virtual ~Window() = default;

	Window(const Window &) = delete;
	Window(Window &&) noexcept = default;

	Window& operator=(const Window &) = delete;
	Window& operator=(Window &&) noexcept = default;

	WindowParameters& Parameters();
	Entity Surface();

protected:

	Entity layout_;
	Entity surface_;
	WindowParameters windowParams_;

};
