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
#include "ReSDL/AudioDevice.h"
#include "ReSDL/Surface.h"
#include "ReSDL/Window.h"
#include "ReSDL/Renderer.h"
#include "ReSDL/Texture.h"
#include "ReSDL/Joystick.h"
#include "ReSDL/GameController.h"

namespace ReSDL {
	
	
	class SDL {
	public:
		SDL(int initFlags = SDL_INIT_EVERYTHING);
		~SDL();
	};
	
	
	
	
	
} // namespace SDL

