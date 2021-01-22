namespace ReSDL {


	struct Surface
	{
		const sdl_handle<SDL_Surface> handle;

		Surface(Uint32 flags,
			int    width,
			int    height,
			int    depth,
			Uint32 Rmask = 0,
			Uint32 Gmask = 0,
			Uint32 Bmask = 0,
			Uint32 Amask = 0)
			: handle(SDL_CreateRGBSurface(flags, width, height, depth, Rmask, Gmask, Bmask, Amask), SDL_FreeSurface)
		{
		}

		Surface(void* pixels,
			int width,
			int height,
			int depth,
			int pitch,
			Uint32 Rmask = 0,
			Uint32 Gmask = 0,
			Uint32 Bmask = 0,
			Uint32 Amask = 0)
			: handle(SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch, Rmask, Gmask, Bmask, Amask), SDL_FreeSurface)
		{
		}

		void setColorMod(Uint8 r, Uint8 g, Uint8 b) {
			check(SDL_SetSurfaceColorMod(handle.get(), r, g, b));
		}

		void setAlphaMod(Uint8 alpha) {
			check(SDL_SetSurfaceAlphaMod(handle.get(), alpha));
		}

		void setBlendMode(SDL_BlendMode blendMode) {
			check(SDL_SetSurfaceBlendMode(handle.get(), blendMode));
		}

		bool mustLock() const {
			return SDL_MUSTLOCK(handle.get()) == SDL_TRUE;
		}

		int getWidth() const {
			return handle->w;
		}

		int getHeight() const {
			return handle->h;
		}
	};

}