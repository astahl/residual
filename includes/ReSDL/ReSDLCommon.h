//
//  MySDLCommon.h
//  libsdlTest
//
//  Created by Andreas Stahl on 04.02.15.
//  Copyright (c) 2015 Andreas. All rights reserved.
//

#pragma once

#include <memory>
#include <string>

#include "SDL.h"
#include <stdexcept>

namespace ReSDL {
	
	class SDLError : std::runtime_error {
	public:
		SDLError() 
		: std::runtime_error(SDL_GetError())
		{}

		SDLError(const std::string& what)
			: std::runtime_error(what + " SDL_GetError: " + SDL_GetError())
		{}
	};


	template<typename T, void (*Deleter)(T*) = nullptr>
	using sdl_handle = std::unique_ptr<T, decltype(Deleter)>;


	template<typename R, void (*Deleter)(R*)>
	class Wrapper {
		sdl_handle<R> m_handle;
	public:
		Wrapper()
			: m_handle{ nullptr, nullptr }
		{

		}

		Wrapper(R* rawPointer, bool own = true)
		: m_handle(rawPointer, own ? Deleter : nullptr) {
			if(!m_handle) {
				throw SDLError{ "Wrapper failed, handle is nullptr." };
			}
		}
	
		Wrapper(const Wrapper&) = delete;

		Wrapper(Wrapper&& other)
			: m_handle{nullptr, nullptr}
		{
			m_handle.swap(other.m_handle);
		}
	
		Wrapper& operator=(Wrapper&& other)
		{
			if (this == &other) {
				return *this;
			}
			m_handle.swap(other.m_handle);
			return *this;
		}

		operator R*() {
			return m_handle.get();
		}

		operator R& () {
			return *m_handle.get();
		}

		~Wrapper() {};
	
		R* raw() const { return m_handle.get(); }
	
		bool isNull() const { return nullptr == m_handle.get(); }
	};
	
	inline void check(int result) {
		if(result != 0) {
			throw SDLError{};
		}
	}
	
}
