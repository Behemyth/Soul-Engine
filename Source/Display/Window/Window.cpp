#include "Window.h"

Window::Window(const WindowParameters& params) :
	layout_(),
	windowParams_(params)
{
	//std::make_unique<SingleLayout>();
}

WindowParameters& Window::Parameters()
{

	return windowParams_;

}

Entity Window::Surface()
{

	return surface_;

}