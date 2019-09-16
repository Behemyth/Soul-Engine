#pragma once

#include "Display/Window/Monitor.h"
#include "Core/Utility/ID/TypeID.h"

class GLFWWindow;
struct GLFWmonitor;
struct GLFWvidmode;

class GLFWMonitor final : public Monitor, TypeID<GLFWMonitor> {

public:
	friend class GLFWWindow;

	GLFWMonitor(GLFWmonitor*);
	~GLFWMonitor() override;

	void Scale(float&, float&) const;

	void Position(int&, int&) const;

	void Size(int&, int&) const;
	void ColorBits(int&, int&, int&) const;
	void RefreshRate(int&) const;

	std::string Name() const;

private:

	GLFWmonitor* monitor_;

	const GLFWvidmode* videoMode_;
	const char* name_;
};
