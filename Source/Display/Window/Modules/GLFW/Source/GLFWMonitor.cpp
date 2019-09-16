#include "GLFWMonitor.h"

#include <GLFW/glfw3.h>

GLFWMonitor::GLFWMonitor(GLFWmonitor* monitor)
	: monitor_(monitor),
	videoMode_(glfwGetVideoMode(monitor)),
	name_(glfwGetMonitorName(monitor))
{
}

GLFWMonitor::~GLFWMonitor()
{
}

void GLFWMonitor::Scale(float& xscale, float& yscale) const
{
	glfwGetMonitorContentScale(monitor_, &xscale, &yscale);
}

void GLFWMonitor::Position(int& xpos, int& ypos) const
{
	glfwGetMonitorPos(monitor_, &xpos, &ypos);
}

void GLFWMonitor::Size(int& width, int& height) const
{
	width = videoMode_->width;
	height = videoMode_->height;
}

void GLFWMonitor::ColorBits(int& red, int& green, int& blue) const
{
	red = videoMode_->redBits;
	green = videoMode_->greenBits;
	blue = videoMode_->blueBits;
}

void GLFWMonitor::RefreshRate(int& refreshRate) const
{
	refreshRate = videoMode_->refreshRate;
}

std::string GLFWMonitor::Name() const
{
	return std::string(name_);
}
