module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

export module synodic.soul.window.backend.sdl;

import std;

import synodic.soul.window;
import synodic.soul.backend.sdl;

export class SDLWindow : public Window<SDLWindow>
{
public:
	explicit SDLWindow(const WindowParameters& params) :
		Window<SDLWindow>(params),
		sdlWindow_(nullptr)
	{
	}

	~SDLWindow()
	{
		Destroy();
	}

	SDLWindow(const SDLWindow&) = delete;

	SDLWindow(SDLWindow&& other) noexcept :
		Window<SDLWindow>(std::move(other)),
		sdlWindow_(other.sdlWindow_)
	{
		other.sdlWindow_ = nullptr;
	}

	SDLWindow& operator=(const SDLWindow&) = delete;

	SDLWindow& operator=(SDLWindow&& other) noexcept
	{
		if (this != &other)
		{
			Destroy();
			Window<SDLWindow>::operator=(std::move(other));
			sdlWindow_		 = other.sdlWindow_;
			other.sdlWindow_ = nullptr;
		}
		return *this;
	}

	SDL_Window* GetNativeHandle()
	{
		return sdlWindow_;
	}

	const SDL_Window* GetNativeHandle() const
	{
		return sdlWindow_;
	}

	bool IsValid() const
	{
		return sdlWindow_ != nullptr;
	}

	void SetNativeHandle(SDL_Window* window)
	{
		Destroy();
		sdlWindow_ = window;
	}

private:
	void Destroy()
	{
		if (sdlWindow_)
		{
			SDL_DestroyWindow(sdlWindow_);
			sdlWindow_ = nullptr;
		}
	}

	SDL_Window* sdlWindow_;
};

export class SDLWindowBackend : public WindowModule<SDLWindow>
{
public:
	SDLWindowBackend(SDLBackend& sdlBackend) :
		WindowModule<SDLWindow>(),
		sdlBackend_(sdlBackend)
	{
	}

	~SDLWindowBackend() override = default;

	SDLWindowBackend(const SDLWindowBackend&) = delete;
	SDLWindowBackend(SDLWindowBackend&&)	  = default;

	SDLWindowBackend& operator=(const SDLWindowBackend&)	 = delete;
	SDLWindowBackend& operator=(SDLWindowBackend&&) noexcept = default;

	void Update() override
	{
		// Events are handled by input backend during Poll()
		// This method can be used for other window updates if needed
	}

	// Called by SDLInputBackend to forward window events
	void HandleEvent(const SDL_Event& event)
	{
		switch (event.type)
		{
			case SDL_EVENT_QUIT :
				active_ = false;
				break;

			case SDL_EVENT_WINDOW_CLOSE_REQUESTED :
				// Find which window closed and remove it
				for (std::size_t i = 0; i < windows_.size(); ++i)
				{
					if (windows_[i].GetNativeHandle() &&
						SDL_GetWindowID(windows_[i].GetNativeHandle()) == event.window.windowID)
					{
						DestroyWindow(i);
						break;
					}
				}
				break;

			default :
				break;
		}
	}

	std::span<const char*> GetRasterExtensions() override
	{
		if (vulkanExtensions_.empty())
		{
			std::uint32_t count = 0;
			const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&count);
			if (extensions && count > 0)
			{
				vulkanExtensions_.assign(extensions, extensions + count);
			}
		}
		return vulkanExtensions_;
	}

protected:
	void OnWindowCreated(SDLWindow& window) override
	{
		SDL_Window* sdlWindow = CreateSDLWindow(window.Parameters());
		window.SetNativeHandle(sdlWindow);
	}

	void OnWindowDestroyed(SDLWindow& window) override
	{
		// SDL window is automatically destroyed in SDLWindow destructor
	}

private:
	SDL_Window* CreateSDLWindow(const WindowParameters& params)
	{
		SDL_WindowFlags flags = SDL_WINDOW_VULKAN;	// Default to Vulkan support

		switch (params.type)
		{
			case WindowType::FULLSCREEN :
				flags |= SDL_WINDOW_FULLSCREEN;
				break;
			case WindowType::BORDERLESS :
				flags |= SDL_WINDOW_BORDERLESS;
				break;
			case WindowType::WINDOWED :
			default :
				// No additional flags
				break;
		}

		SDL_Window* window = SDL_CreateWindow(
			params.title.c_str(),
			static_cast<int>(params.pixelSize.x),
			static_cast<int>(params.pixelSize.y),
			flags);

		if (window && (params.type == WindowType::WINDOWED || params.type == WindowType::BORDERLESS))
		{
			SDL_SetWindowPosition(
				window,
				static_cast<int>(params.pixelPosition.x),
				static_cast<int>(params.pixelPosition.y));
		}

		return window;
	}

	SDLBackend& sdlBackend_;
	std::vector<const char*> vulkanExtensions_;
};
