#include "GLFWInputBackend.h"

#include "Core/Utility/Exception/Exception.h"
#include "Display/Window/Modules/GLFW/GLFWWindow.h"
#include "Display/Window/Modules/GLFW/GLFWWindowBackend.h"

#include <GLFW/glfw3.h>

#include <cassert>

GLFWInputBackend::GLFWInputBackend() : mouseXOffset_(0), mouseYOffset_(0), mouseXPos_(0), mouseYPos_(0)
{
}

void GLFWInputBackend::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (action == GLFW_PRESS) {

		//if the state was previously open, start the timer
		if (buttonStates_[key].state == ButtonState::OPEN) {
			buttonStates_[key].sincePress.Reset();
		}

		buttonStates_[key].state = ButtonState::PRESS;

	}
	else if (action == GLFW_REPEAT) {

		//GLFW_REPEAT is handled in GLFW, but we want custimization for input, so this is ignored

	}
	else if (action == GLFW_RELEASE) {

		buttonStates_[key].state = ButtonState::RELEASE;

	}
	else {

		throw NotImplemented();

	}

}

void GLFWInputBackend::CharacterCallback(GLFWwindow* window, uint codepoint) {

}

void GLFWInputBackend::ModdedCharacterCallback(GLFWwindow* window, uint, int) {

}

void GLFWInputBackend::ButtonCallback(GLFWwindow* window, int button, int action, int mods) {

	assert(button >= 0);

	for (const auto& callback : mouseButtonCallbacks_) {

		switch(action) {
			case GLFW_PRESS:
				callback(button, ButtonState::PRESS);
				break;

			case GLFW_RELEASE:
				callback(button, ButtonState::OPEN);
		}

	}

}

void GLFWInputBackend::CursorCallback(GLFWwindow* window, double xPos, double yPos) {

	mouseXOffset_ = xPos - mouseXPos_;
	mouseYOffset_ = yPos - mouseYPos_;
	mouseXPos_ = xPos;
	mouseYPos_ = yPos;

	for (const auto& callback : mousePositionCallbacks_) {

		callback(xPos, yPos);

	}

}

void GLFWInputBackend::CursorEnterCallback(GLFWwindow* window, int) {

}
void GLFWInputBackend::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {

}


void GLFWInputBackend::Listen(Window& window)
{

	//Luxery of having the Windowing system be GLFW
	const auto glfwWindow = static_cast<GLFWWindow*>(&window);
	GLFWwindow* context = glfwWindow->Context();

	glfwSetKeyCallback(
		context, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			const auto userPointers =
				static_cast<GLFWWindowBackend::UserPointers*>(glfwGetWindowUserPointer(window));
			userPointers->inputBackend->KeyCallback(window, key, scancode, action, mods);
		});

	glfwSetCharCallback(context, [](GLFWwindow* window, uint codepoint) {
		const auto userPointers =
			static_cast<GLFWWindowBackend::UserPointers*>(glfwGetWindowUserPointer(window));
		userPointers->inputBackend->CharacterCallback(window, codepoint);
	});

	glfwSetCharModsCallback(context, [](GLFWwindow* window, uint a, int b) {
		const auto userPointers =
			static_cast<GLFWWindowBackend::UserPointers*>(glfwGetWindowUserPointer(window));
		userPointers->inputBackend->ModdedCharacterCallback(window, a, b);
	});

	glfwSetMouseButtonCallback(
		context, [](GLFWwindow* window, int button, int action, int mods) {
		const auto userPointers =
			static_cast<GLFWWindowBackend::UserPointers*>(glfwGetWindowUserPointer(window));
			userPointers->inputBackend->ButtonCallback(window, button, action, mods);
		});

	glfwSetCursorPosCallback(context, [](GLFWwindow* window, double xPos, double yPos) {
		const auto userPointers =
			static_cast<GLFWWindowBackend::UserPointers*>(glfwGetWindowUserPointer(window));
		userPointers->inputBackend->CursorCallback(window, xPos, yPos);
	});

	glfwSetCursorEnterCallback(context, [](GLFWwindow* window, int temp) {
		const auto userPointers =
			static_cast<GLFWWindowBackend::UserPointers*>(glfwGetWindowUserPointer(window));
		userPointers->inputBackend->CursorEnterCallback(window, temp);
	});

	glfwSetScrollCallback(context, [](GLFWwindow* window, double xoffset, double yoffset) {
		const auto userPointers =
			static_cast<GLFWWindowBackend::UserPointers*>(glfwGetWindowUserPointer(window));
		userPointers->inputBackend->ScrollCallback(window, xoffset, yoffset);
	});

}

bool GLFWInputBackend::Poll() {

	//process all events (includes Window events)
	glfwPollEvents();

	//TODO break into tasks 
	//fire off all key events and their logic
	for (auto&[keyID, key] : buttonStates_) {

		if (key.state != ButtonState::OPEN) {

			//change state if the key timer is beyond the requested.
			if (key.state == ButtonState::PRESS &&
				key.sincePress.Elapsed() > key.timeToRepeat) {

				key.state = ButtonState::REPEAT;

			}

			//TODO implement inputsets
			const auto inputSetOffset = 0 * 350; //350+ are not defined in GLFW ButtonStates
			//eventManager_->Emit("Input"_hashed, keyID + inputSetOffset, key.state);

			//handle reset case after emitting a release event
			if (key.state == ButtonState::RELEASE) {
				key.state = ButtonState::OPEN;
			}

		}
	}

	//emit mouse tasks
	//eventManager_->Emit("Input"_hashed, "Mouse Position"_hashed, mouseXOffset_, mouseYOffset_);

	//zero offsets
	mouseXOffset_ = 0.0;
	mouseYOffset_ = 0.0;

	return true;
}
