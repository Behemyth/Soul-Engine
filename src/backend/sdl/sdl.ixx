module;

#include <SDL3/SDL_init.h>

export module synodic.soul.backend.sdl;


export class SDLBackend
{
public:
	SDLBackend()
	{
		// TODO: Must be called on the main thread
		constexpr SDL_InitFlags flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD;
		bool result = SDL_Init(flags);

		// TODO: Handle error
	}

	~SDLBackend()
	{
		SDL_Quit();
	}
};
