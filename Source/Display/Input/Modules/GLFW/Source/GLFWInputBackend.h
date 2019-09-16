#pragma once

#include "Display/Input/Button.h"
#include "Display/Input/InputModule.h"

#include <unordered_map>

struct GLFWwindow;

class GLFWInputBackend : public InputModule {

public:

	GLFWInputBackend();
	~GLFWInputBackend() override = default;

	GLFWInputBackend(const GLFWInputBackend&) = delete;
	GLFWInputBackend(GLFWInputBackend&&) = default;

	GLFWInputBackend& operator=(const GLFWInputBackend&) = delete;
	GLFWInputBackend& operator=(GLFWInputBackend&&) noexcept = default;


	bool Poll() override;
	void Listen(Window&) override;

	void KeyCallback(GLFWwindow*, int, int, int, int);
	void CharacterCallback(GLFWwindow*, uint);
	void ModdedCharacterCallback(GLFWwindow*, uint, int);
	void ButtonCallback(GLFWwindow*, int, int, int);
	void CursorCallback(GLFWwindow*, double, double);
	void CursorEnterCallback(GLFWwindow*, int);
	void ScrollCallback(GLFWwindow*, double, double);


private:

	std::unordered_map<int, Button> buttonStates_;

	double mouseXPos_;
	double mouseYPos_;

	double mouseXOffset_;
	double mouseYOffset_;


};
