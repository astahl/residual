namespace ReSDL {

	struct GameController 
	{
		sdl_handle<SDL_GameController> handle;

		GameController(int device_index)
			: handle(SDL_GameControllerOpen(device_index), SDL_GameControllerClose)
		{
		}

		std::string name() const
		{
			return SDL_GameControllerName(handle.get());
		}

		std::string mapping() const
		{
			return SDL_GameControllerMapping(handle.get());
		}

		Sint16 getAxis(SDL_GameControllerAxis axis) const
		{
			return SDL_GameControllerGetAxis(handle.get(), axis);
		}

		Uint8 getButton(SDL_GameControllerButton button) const
		{
			return SDL_GameControllerGetButton(handle.get(), button);
		}

		SDL_Joystick* getJoystick() const
		{
			return SDL_GameControllerGetJoystick(handle.get());
		}

		static bool isGameController(int joystick_index)
		{
			return SDL_IsGameController(joystick_index);
		}
	};

}