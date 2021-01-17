//
//  ReSDLCore.h
//  libsdlTest
//
//  Created by Andreas Stahl on 23.02.15.
//  Copyright (c) 2015 Andreas. All rights reserved.
//
#pragma once

#include "SDL.h"
#include "ReSDL/ReSDLTypes.h"
#include "ReSDL/ReSDLCommon.h"


#include <optional>

namespace ReSDL {
	
	
	class SDL {
	public:
		SDL(int initFlags = SDL_INIT_EVERYTHING | SDL_INIT_AUDIO);
		~SDL();
	};
	
	
	class Window : public Wrapper<SDL_Window, SDL_DestroyWindow>
	{
	public:
		Window(const char *title,
					 int x, int y, int w,
					 int h, Uint32 flags)
		: Wrapper(SDL_CreateWindow(title, x, y, w, h, flags))
		{
		}
		
		void getSize(int *w, int *h)
		{
			SDL_GetWindowSize(raw(), w, h);
		}
		
		void getDrawableSize(int *w, int *h)
		{
			SDL_GL_GetDrawableSize(raw(), w, h);
		}
		
		SDL_Surface *getSurface()
		{
			return SDL_GetWindowSurface(raw());
		}
	};
	
	
	class Renderer : public Wrapper<SDL_Renderer, SDL_DestroyRenderer>
	{
	public:
		Renderer(SDL_Window* window,
						 int         index,
						 Uint32      flags)
		: Wrapper(SDL_CreateRenderer(window, index, flags))
		{
		}
		
		void drawRect(const SDL_Rect *rect) {
			check(SDL_RenderDrawRect(raw(), rect));
		}
		
		void fillRect(const SDL_Rect *rect) {
			check(SDL_RenderFillRect(raw(), rect));
		}
		
		void drawPoints(const SDL_Point *points, size_t count) {
			check(SDL_RenderDrawPoints(raw(), points, static_cast<int>(count)));
		}
		
		void drawLine(int x1, int y1, int x2, int y2) {
			check(SDL_RenderDrawLine(raw(), x1, y1, x2, y2));
		}
		
		void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
			check(SDL_SetRenderDrawColor(raw(), r, g, b, a));
		}
		
		void getDrawColor(Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
			check(SDL_GetRenderDrawColor(raw(), r, g, b, a));
		}
		
		void setDrawColor(const Color &c) {
			this->setDrawColor(c.r, c.g, c.b, c.a);
		}
		
		void copy(SDL_Texture& texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect) const {
			check(SDL_RenderCopy(raw(), &texture, srcrect, dstrect));
		}
		
		void copyEx(SDL_Texture& texture,
								const SDL_Rect * srcrect,
								const SDL_Rect * dstrect,
								const double angle,
								const SDL_Point *center = nullptr,
								const SDL_RendererFlip flip = SDL_FLIP_NONE) const {
			check(SDL_RenderCopyEx(raw(), &texture, srcrect, dstrect, angle, center, flip));
		}
		
		void clear() {
			check(SDL_RenderClear(raw()));
		}
		
		void present() {
			SDL_RenderPresent(raw());
		}
		
		void setLogicalSize(int w, int h) {
			check(SDL_RenderSetLogicalSize(raw(), w, h));
		}
		
		void setRenderTarget(SDL_Texture *texture) {
			check(SDL_SetRenderTarget(raw(), texture));
		}
		
		SDL_Texture *getRenderTarget() const {
			return SDL_GetRenderTarget(raw());
		}
		
		bool isRenderTargetSupported() const {
			return SDL_TRUE == SDL_RenderTargetSupported(raw());
		}
		
		void setViewport(const SDL_Rect *rect) {
			check(SDL_RenderSetViewport(raw(), rect));
		}
		
		void setDrawBlendMode(SDL_BlendMode blendMode) {
			check(SDL_SetRenderDrawBlendMode(raw(), blendMode));
		}
	};
	
	
	class Surface : public Wrapper<SDL_Surface, SDL_FreeSurface>
	{
	public:
		Surface(Uint32 flags,
						int    width,
						int    height,
						int    depth,
						Uint32 Rmask = 0,
						Uint32 Gmask = 0,
						Uint32 Bmask = 0,
						Uint32 Amask = 0)
		: Wrapper(SDL_CreateRGBSurface(flags, width, height, depth, Rmask, Gmask, Bmask, Amask))
		{
		}
		
		Surface(void *pixels,
						int width,
						int height,
						int depth,
						int pitch,
						Uint32 Rmask = 0,
						Uint32 Gmask = 0,
						Uint32 Bmask = 0,
						Uint32 Amask = 0)
		: Wrapper(SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch, Rmask, Gmask, Bmask, Amask))
		{
		}
		
		void setColorMod(Uint8 r, Uint8 g, Uint8 b) {
			check(SDL_SetSurfaceColorMod(raw(), r, g, b));
		}
		
		void setAlphaMod(Uint8 alpha) {
			check(SDL_SetSurfaceAlphaMod(raw(), alpha));
		}
		
		void setBlendMode(SDL_BlendMode blendMode) {
			check(SDL_SetSurfaceBlendMode(raw(), blendMode));
		}
		
		bool mustLock() const {
			return SDL_MUSTLOCK(raw()) == SDL_TRUE;
		}
		
		int getWidth() const {
			return raw()->w;
		}
		
		int getHeight() const {
			return raw()->h;
		}
	};
	
	
	class Texture : public Wrapper<SDL_Texture, SDL_DestroyTexture>
	{
	public:
		Texture() : Wrapper() {}

		Texture(Renderer &renderer,
						Uint32        format,
						int           access,
						int           w,
						int           h)
		: Wrapper(SDL_CreateTexture(renderer, format, access, w, h)),
			m_format(format), m_access(access), m_w(w), m_h(h)

		{
		}
	
		Texture(Renderer& renderer,
				Surface& surface)
		: Wrapper(SDL_CreateTextureFromSurface(renderer, surface))
		{
			SDL_QueryTexture(raw(), &m_format, &m_access, &m_w, &m_h);
		}
		
		void setColorMod(Uint8 r, Uint8 g, Uint8 b) {
			check(SDL_SetTextureColorMod(raw(), r, g, b));
		}
		
		void setAlphaMod(Uint8 alpha) {
			check(SDL_SetTextureAlphaMod(raw(), alpha));
		}
		
		void setBlendMode(SDL_BlendMode blendMode) {
			check(SDL_SetTextureBlendMode(raw(), blendMode));
		}
		
		SDL_Point getSize() const {
			return {m_w, m_h};
		}
		
		int getFormat() const {
			return m_format;
		}
		
		int getAccess() const {
			return m_access;
		}
	
	private:
		Uint32 m_format = 0;
		int m_access = 0;
		int m_w = 0;
		int m_h = 0;
	};
	
	
	class Joystick : public Wrapper<SDL_Joystick, SDL_JoystickClose>
	{
	public:
		Joystick(int device_index)
		: Wrapper(SDL_JoystickOpen(device_index))
		{
		}
		
		std::string name() const
		{
			return SDL_JoystickName(raw());
		}
		
		SDL_JoystickID instanceId() const
		{
			return SDL_JoystickInstanceID(raw());
		}
		
		Sint16 getAxis(int axis) const
		{
			return SDL_JoystickGetAxis(raw(), axis);
		}
		
		int numAxes() const
		{
			return SDL_JoystickNumAxes(raw());
		}
		
		Uint8 getButton(int button) const
		{
			return SDL_JoystickGetButton(raw(), button);
		}
		
		int numButtons() const
		{
			return SDL_JoystickNumButtons(raw());
		}
		
		Uint8 getHat(int hat) const
		{
			return SDL_JoystickGetHat(raw(), hat);
		}
		
		int numHats() const
		{
			return SDL_JoystickNumHats(raw());
		}
		//--------------------------------------------------------------------------
		
		static std::string nameForIndex(int device_index)
		{
			return SDL_JoystickNameForIndex(device_index);
		}
		
		static int numJoysticks()
		{
			return SDL_NumJoysticks();
		}
		
	};
	
	class GameController : public Wrapper<SDL_GameController, SDL_GameControllerClose>
	{
	public:
		GameController(int device_index)
		: Wrapper(SDL_GameControllerOpen(device_index))
		{
		}
		
		std::string name() const
		{
			return SDL_GameControllerName(raw());
		}
		
		std::string mapping() const
		{
			return SDL_GameControllerMapping(raw());
		}
		
		Sint16 getAxis(SDL_GameControllerAxis axis) const
		{
			return SDL_GameControllerGetAxis(raw(), axis);
		}
		
		Uint8 getButton(SDL_GameControllerButton button) const
		{
			return SDL_GameControllerGetButton(raw(), button);
		}
		
		SDL_Joystick *getJoystick() const
		{
			return SDL_GameControllerGetJoystick(raw());
		}
		
		static bool isGameController(int joystick_index)
		{
			return SDL_IsGameController(joystick_index);
		}
	};
	
} // namespace SDL

