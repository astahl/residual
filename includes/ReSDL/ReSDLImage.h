//
//  MySDLImage.h
//  libsdlTest
//
//  Created by Andreas Stahl on 04.02.15.
//  Copyright (c) 2015 Andreas. All rights reserved.
//

#pragma once

#include "ReSDLCommon.h"

#include <SDL_image.h>

namespace ReSDL {
	
	
	class Image {
	public:
		Image(IMG_InitFlags flags = IMG_InitFlags(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP))
		{
			IMG_Init(flags);
		}
		
		~Image()
		{
			IMG_Quit();
		}
	};


}
