namespace ReSDL {

	struct Texture;

	struct Renderer
	{
		sdl_handle<SDL_Renderer> handle;

		Renderer(Window& window,
			int         index,
			Uint32      flags)
			: handle(SDL_CreateRenderer(window.handle.get(), index, flags), SDL_DestroyRenderer)
		{
		}

		void drawRect(const SDL_Rect& rect) {
			check(SDL_RenderDrawRect(handle.get(), &rect));
		}

		void drawEntireRenderTarget() {
			check(SDL_RenderDrawRect(handle.get(), nullptr));
		}

		void fillRect(const SDL_Rect& rect) {
			check(SDL_RenderFillRect(handle.get(), &rect));
		}

		void fillEntireRenderTarget() {
			check(SDL_RenderFillRect(handle.get(), nullptr));
		}

		void drawPoints(const SDL_Point* points, size_t count) {
			check(SDL_RenderDrawPoints(handle.get(), points, static_cast<int>(count)));
		}

		template<class Collection>
		void drawPoints(const Collection& points) {
			check(SDL_RenderDrawPoints(handle.get(), &points[0], points.size()));
		}

		void drawLine(int x1, int y1, int x2, int y2) {
			check(SDL_RenderDrawLine(handle.get(), x1, y1, x2, y2));
		}

		void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
			check(SDL_SetRenderDrawColor(handle.get(), r, g, b, a));
		}

		void getDrawColor(Uint8* r, Uint8* g, Uint8* b, Uint8* a) {
			check(SDL_GetRenderDrawColor(handle.get(), r, g, b, a));
		}

		Color getDrawColor() {
			Color c{};
			check(SDL_GetRenderDrawColor(handle.get(), &c.r, &c.g, &c.b, &c.a));
			return c;
		}

		void setDrawColor(const Color& c) {
			this->setDrawColor(c.r, c.g, c.b, c.a);
		}

		void copy(SDL_Texture& texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect) const {
			check(SDL_RenderCopy(handle.get(), &texture, srcrect, dstrect));
		}

		void copyEx(SDL_Texture& texture,
			const SDL_Rect* srcrect,
			const SDL_Rect* dstrect,
			const double angle,
			const SDL_Point* center = nullptr,
			const SDL_RendererFlip flip = SDL_FLIP_NONE) const {
			check(SDL_RenderCopyEx(handle.get(), &texture, srcrect, dstrect, angle, center, flip));
		}

		void clear() {
			check(SDL_RenderClear(handle.get()));
		}

		void present() {
			SDL_RenderPresent(handle.get());
		}

		void setLogicalSize(int w, int h) {
			check(SDL_RenderSetLogicalSize(handle.get(), w, h));
		}

		void setRenderTarget(SDL_Texture* texture) {
			check(SDL_SetRenderTarget(handle.get(), texture));
		}

		SDL_Texture* getRenderTarget() const {
			return SDL_GetRenderTarget(handle.get());
		}

		bool isRenderTargetSupported() const {
			return SDL_TRUE == SDL_RenderTargetSupported(handle.get());
		}

		void setViewport(const SDL_Rect& rect) {
			check(SDL_RenderSetViewport(handle.get(), &rect));
		}

		void setViewportToEntireTarget() {
			check(SDL_RenderSetViewport(handle.get(), nullptr));
		}

		void setDrawBlendMode(SDL_BlendMode blendMode) {
			check(SDL_SetRenderDrawBlendMode(handle.get(), blendMode));
		}
	};
}