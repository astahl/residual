namespace ReSDL {


	struct Texture
	{
		sdl_handle<SDL_Texture> handle;

		Uint32 format = 0;
		int access = 0;
		ReSDL::Size size{};

		Texture(Renderer& renderer,
			SDL_PixelFormatEnum format,
			int           access,
			int           w,
			int           h)
			: handle(SDL_CreateTexture(renderer.handle.get(), format, access, w, h), SDL_DestroyTexture)
			, format(format)
			, access(access)
			, size{ w, h }
		{
		}

		Texture(Renderer& renderer,
			Surface& surface)
			: handle(SDL_CreateTextureFromSurface(renderer.handle.get(), surface.handle.get()), SDL_DestroyTexture)
		{
			SDL_QueryTexture(handle.get(), &format, &access, &size.width, &size.height);
		}

		void setColorMod(Uint8 r, Uint8 g, Uint8 b) {
			check(SDL_SetTextureColorMod(handle.get(), r, g, b));
		}

		void setAlphaMod(Uint8 alpha) {
			check(SDL_SetTextureAlphaMod(handle.get(), alpha));
		}

		void setBlendMode(SDL_BlendMode blendMode) {
			check(SDL_SetTextureBlendMode(handle.get(), blendMode));
		}

		SDL_Texture& operator*() {
			return *handle;
		}

	};

}