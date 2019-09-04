#pragma once

#include "Display/Window/Window.h"
#include "Core/Utility/ID/TypeID.h"

#include <memory>

struct GLFWmonitor;
struct GLFWwindow;
class GLFWMonitor;
class RasterModule;
class RasterBackend;

class GLFWWindow final : public Window, TypeID<GLFWWindow> {

public:

	GLFWWindow(const WindowParameters&, GLFWMonitor&, std::shared_ptr<RasterModule>, bool);
	~GLFWWindow() override;

	GLFWWindow(const GLFWWindow &) = delete;
	GLFWWindow(GLFWWindow &&) noexcept = default;

	GLFWWindow& operator=(const GLFWWindow &) = delete;
	GLFWWindow& operator=(GLFWWindow &&) noexcept = default;


	void FrameBufferResize(int, int);


	GLFWwindow* Context() const;
	bool Master() const;


private:

	std::shared_ptr<RasterModule> rasterModule_;

	GLFWwindow* context_;

	bool master_;

};
