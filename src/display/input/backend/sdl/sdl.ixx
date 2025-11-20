module;

#include <SDL3/SDL.h>

export module synodic.soul.input.backend.sdl;

import std;

import synodic.soul.input;
import synodic.soul.backend.sdl;
import synodic.soul.window.backend.sdl;


export class SDLInputBackend : public InputModule
{
public:
	SDLInputBackend(SDLBackend& sdlBackend, SDLWindowBackend& windowBackend) :
		InputModule(),
		sdlBackend_(sdlBackend),
		windowBackend_(&windowBackend),
		mouseX_(0.0),
		mouseY_(0.0)
	{
	}

	~SDLInputBackend() override = default;

	SDLInputBackend(const SDLInputBackend&) = delete;
	SDLInputBackend(SDLInputBackend&&) = default;

	SDLInputBackend& operator=(const SDLInputBackend&) = delete;
	SDLInputBackend& operator=(SDLInputBackend&&) noexcept = default;

	bool Poll() override
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (IsInputEvent(event))
			{
				HandleInputEvent(event);
			}
			else if (windowBackend_)
			{
				windowBackend_->HandleEvent(event);
			}
		}
		return true;
	}

	double GetMouseX() const { return mouseX_; }
	double GetMouseY() const { return mouseY_; }

private:
	bool IsInputEvent(const SDL_Event& event) const
	{
		switch (event.type)
		{
			case SDL_EVENT_MOUSE_MOTION:
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
			case SDL_EVENT_MOUSE_WHEEL:
			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
			case SDL_EVENT_TEXT_INPUT:
			case SDL_EVENT_TEXT_EDITING:
			case SDL_EVENT_GAMEPAD_ADDED:
			case SDL_EVENT_GAMEPAD_REMOVED:
			case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			case SDL_EVENT_GAMEPAD_BUTTON_UP:
				return true;
			default:
				return false;
		}
	}

	void HandleInputEvent(const SDL_Event& event)
	{
		switch (event.type)
		{
			case SDL_EVENT_MOUSE_MOTION:
				HandleMouseMotion(event.motion);
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				HandleMouseButton(event.button, ButtonState::PRESS);
				break;

			case SDL_EVENT_MOUSE_BUTTON_UP:
				HandleMouseButton(event.button, ButtonState::RELEASE);
				break;

			case SDL_EVENT_KEY_DOWN:
				HandleKeyboard(event.key, ButtonState::PRESS);
				break;

			case SDL_EVENT_KEY_UP:
				HandleKeyboard(event.key, ButtonState::RELEASE);
				break;

			case SDL_EVENT_MOUSE_WHEEL:
				// TODO: Implement mouse wheel handling
				break;

			case SDL_EVENT_TEXT_INPUT:
			case SDL_EVENT_TEXT_EDITING:
				// TODO: Implement text input handling
				break;

			case SDL_EVENT_GAMEPAD_ADDED:
			case SDL_EVENT_GAMEPAD_REMOVED:
			case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			case SDL_EVENT_GAMEPAD_BUTTON_UP:
				// TODO: Implement gamepad handling
				break;

			default:
				break;
		}
	}

	void HandleMouseMotion(const SDL_MouseMotionEvent& motion)
	{
		mouseX_ = static_cast<double>(motion.x);
		mouseY_ = static_cast<double>(motion.y);

		for (auto& callback : mousePositionCallbacks_)
		{
			callback(mouseX_, mouseY_);
		}
	}

	void HandleMouseButton(const SDL_MouseButtonEvent& button, ButtonState state)
	{
		std::uint32_t buttonId = static_cast<std::uint32_t>(button.button);

		for (auto& callback : mouseButtonCallbacks_)
		{
			callback(buttonId, state);
		}
	}

	void HandleKeyboard(const SDL_KeyboardEvent& key, ButtonState state)
	{
		// TODO: Implement keyboard callbacks when added to InputModule
		// For now, just acknowledge the key event
	}

	SDLBackend& sdlBackend_;
	SDLWindowBackend* windowBackend_;
	double mouseX_;
	double mouseY_;
};
