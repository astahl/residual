#include "SDL.h"
#include <optional>

namespace ReSDL {


	struct Window
	{
		using Id = unsigned int;

		const sdl_handle<SDL_Window> handle;
		
		Window(const char* title,
			int x, int y, int w,
			int h, Uint32 flags)
			: handle(SDL_CreateWindow(title, x, y, w, h, flags), SDL_DestroyWindow)
		{
		}

		Id getId() const {
			return SDL_GetWindowID(handle.get());
		}

		Size getSize() const
		{
			int w, h;
			SDL_GetWindowSize(handle.get(), &w, &h);
			return { w, h };
		}

		Size getDrawableSize()
		{
			int w, h;
			SDL_GL_GetDrawableSize(handle.get(), &w, &h);
			return { w, h };
		}

		SDL_Surface* getSurface()
		{
			SDL_Surface* surface = SDL_GetWindowSurface(handle.get());
			return surface;
		}

	};

}