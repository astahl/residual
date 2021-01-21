//
//  ReSDLTypes.h
//  libsdlTest
//
//  Created by Andreas Stahl on 23.02.15.
//  Copyright (c) 2015 Andreas. All rights reserved.
//

#pragma once

#include "ReSDLCommon.h"

namespace ReSDL {
	
	struct Size {
		int width, height;
	};

struct Color {
	uint8_t r, g, b, a;
	
	static const Color Red;
	static const Color Green;
	static const Color DarkBlue;
	static const Color Blue;
	static const Color LightBlue;
	static const Color Yellow;
	static const Color Magenta;
	static const Color Cyan;
	static const Color Black;
	static const Color DarkGrey;
	static const Color Grey;
	static const Color LightGrey;
	static const Color White;
	
	constexpr Color withAlpha(Uint8 alpha) const {
		return {r, g, b, alpha};
	}
	
	template<typename T>
	constexpr Color operator*(const T& scale) const {
		return { static_cast<uint8_t>(scale * r),
			static_cast<uint8_t>(scale * g),
			static_cast<uint8_t>(scale * b),
			static_cast<uint8_t>(scale * a)};
	}
	
	
};

constexpr Color max(const Color& c1, const Color& c2) {
	return { cmax(c1.r, c2.r),
		cmax(c1.g, c2.g),
		cmax(c1.b, c2.b),
		cmax(c1.a, c2.a)};
}
	
	
	inline SDL_Rect MakeRect(const SDL_Point &origin, const SDL_Point &size)
	{
		return {origin.x, origin.y, size.x, size.y};
	}
	
	inline SDL_Rect RectMoveTo(const SDL_Rect &rect, const SDL_Point &destination)
	{
		return {destination.x, destination.y, rect.w, rect.h};
	}
}

constexpr SDL_Point operator+(const SDL_Point &a, const SDL_Point &b)
{
	return {a.x + b.x, a.y + b.y};
}

constexpr SDL_Point operator*(const SDL_Point &a, const SDL_Point &b)
{
	return {a.x * b.x, a.y * b.y};
}

template<typename TScalar>
constexpr SDL_Point operator*(const SDL_Point &a, const TScalar &b)
{
	return {a.x * b, a.y * b};
}

constexpr SDL_Point operator-(const SDL_Point &a, const SDL_Point &b)
{
	return {a.x - b.x, a.y - b.y};
}

constexpr SDL_Point operator%(const SDL_Point &a, const SDL_Point &b)
{
	return {a.x % b.x, a.y % b.y};
}

constexpr bool operator<(const SDL_Point &a, const SDL_Point &b)
{
	return (a.x < b.x) && (a.y < b.y);
}


