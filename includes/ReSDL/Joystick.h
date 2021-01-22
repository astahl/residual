namespace ReSDL {


	struct Joystick
	{
		sdl_handle<SDL_Joystick> handle;
	
		Joystick(int device_index)
			: handle(SDL_JoystickOpen(device_index), SDL_JoystickClose)
		{
		}

		std::string name() const
		{
			return SDL_JoystickName(handle.get());
		}

		SDL_JoystickID instanceId() const
		{
			return SDL_JoystickInstanceID(handle.get());
		}

		Sint16 getAxis(int axis) const
		{
			return SDL_JoystickGetAxis(handle.get(), axis);
		}

		int numAxes() const
		{
			return SDL_JoystickNumAxes(handle.get());
		}

		Uint8 getButton(int button) const
		{
			return SDL_JoystickGetButton(handle.get(), button);
		}

		int numButtons() const
		{
			return SDL_JoystickNumButtons(handle.get());
		}

		Uint8 getHat(int hat) const
		{
			return SDL_JoystickGetHat(handle.get(), hat);
		}

		int numHats() const
		{
			return SDL_JoystickNumHats(handle.get());
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

}