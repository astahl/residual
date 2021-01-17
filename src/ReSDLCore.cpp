#include "ReSDL/ReSDLCore.h"

#include "SDL.h"

namespace ReSDL {

SDL::SDL(int initFlags)
{
	check(SDL_Init(initFlags));
}

SDL::~SDL()
{
	SDL_Quit();
}

}