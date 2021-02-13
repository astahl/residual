#include <Engine/Engine.h>


namespace Engine {
	
	
	RttRendererWindow::RttRendererWindow(int targetWidth, int targetHeight, float pixelAspectRatio)
	: m_TargetWidth(targetWidth)
	, m_TargetHeight(targetHeight)
	, m_AspectRatio(pixelAspectRatio > 0 ? 1 / pixelAspectRatio : static_cast<float>(targetWidth) / static_cast<float>(targetHeight))
		, m_Window{ std::make_shared<ReSDL::Window>("Puup",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			m_TargetWidth,
			m_TargetHeight,
			SDL_WINDOW_OPENGL
			| SDL_WINDOW_ALLOW_HIGHDPI
			| SDL_WINDOW_RESIZABLE
			| SDL_WINDOW_MAXIMIZED) }
		, m_Renderer{ std::make_shared<ReSDL::Renderer>(*m_Window,
			0,
			SDL_RENDERER_ACCELERATED
			| SDL_RENDERER_PRESENTVSYNC
			| SDL_RENDERER_TARGETTEXTURE) }
		, m_TargetTexture{ *m_Renderer, SDL_PIXELFORMAT_UNKNOWN, SDL_TEXTUREACCESS_TARGET, m_TargetWidth, m_TargetHeight }
	{
		updateDstRect();
	}
	
	void RttRendererWindow::prepareFrame()
	{
		m_Renderer->setRenderTarget(m_TargetTexture.handle.get());
	}
	
	void RttRendererWindow::finalizeFrame()
	{
		// render target texture to window
		m_Renderer->setRenderTarget(nullptr);
		m_Renderer->setDrawColor(ReSDL::Color::Black);
		m_Renderer->clear();
		m_Renderer->copy(*m_TargetTexture, nullptr, &m_dstRect);
		m_Renderer->present();
	}

	void RttRendererWindow::updateDstRect()
	{
		// calculate Window geometry to apply letter/pillarboxes as needed
		auto windowSize = m_Window->getDrawableSize();

		int targetXOffset = 0;
		int targetYOffset = 0;
		int targetHeight = windowSize.width / m_AspectRatio;
		int targetWidth = windowSize.width;
		if (targetHeight > windowSize.height)
		{
			targetWidth = windowSize.height * m_AspectRatio;
			targetHeight = windowSize.height;
			targetXOffset = (windowSize.width - targetWidth) / 2;
		}
		else
		{
			targetYOffset = (windowSize.height - targetHeight) / 2;
		}
		m_dstRect = SDL_Rect{ targetXOffset, targetYOffset, targetWidth, targetHeight };
	}
	
	std::shared_ptr<ReSDL::Renderer> RttRendererWindow::renderer()
	{
		return m_Renderer;
	}
	
	Engine::Engine(int width, int height, float pixelAspectRatio)
		: window{ width, height, pixelAspectRatio }
		, frameCount{ 0 }
		, sdl{SDL_INIT_EVERYTHING}
	{
		
	}
	
	
	void Engine::addRenderable(std::shared_ptr<IRenderable> renderable)
	{
		m_Renderables.push_back(renderable);
		std::sort(m_Renderables.begin(),
			m_Renderables.end(),
			[](auto& l, auto& r) {
				return l->order() < r->order();
			});
	}
	
	void Engine::addUpdateable(std::shared_ptr<IUpdatable> updateable)
	{
		m_Updateables.push_back(updateable);
	}
	
	struct AudioData {
		float phase = 0.0;
		float volume = 0.0;
	};

	void Engine::start()
	{
		using namespace ReSDL;
		SDL_AudioSpec spec{}, got{};

		AudioData ad;

		SDL_AudioCallback cb = [](void* userdata, Uint8* stream, int length) {
			auto &ad = *reinterpret_cast<AudioData*>(userdata);
			auto data = reinterpret_cast<float*>(stream);
			size_t count = length / sizeof(float);
			const float freq = 2.0f * static_cast<float>(std::_Pi) * 440.0f;
			const float step = 1.0f / 48000;
			float t = ad.phase;
			for(size_t i = 0; i < count; i += 2) {
				data[i] = data[i + 1] = ad.volume * sinf(t * freq);
				t += step;
			}			
			ad.phase = t;
		};
		spec.callback = cb;
		spec.freq = 48000;
		spec.channels = 2;
		spec.format = AUDIO_F32;
		spec.samples = 256;
		spec.userdata = &ad;
		auto deviceNames = AudioDevice::enumerate(false);
		auto device = AudioDevice::open(deviceNames[0], false, spec, got);

		
		// std::array<float, 24000> data;
		// const float freq = 2.0 * std::_Pi * 240.0;
		// float t = 0;
		// for(size_t i = 0; i < data.size(); i += 2) {
		// 	data[i] = data[i + 1] = 1.0 * sinf(t * freq);
		// 	t += step;
		// }
		// const auto statusBefore = device->status();

		device->unpause();
		
		bool isDone = false;
		// Handle quit events by setting the loop exit variable
		eventManager.handlers[SDL_QUIT] = [&isDone](const SDL_Event&) { isDone = true; };
		
		bool hasFocus = false;
		eventManager.handlers[SDL_WINDOWEVENT] = [&hasFocus, this](const SDL_Event& e) {
			switch (e.window.event)
			{
			// Handle focussing of window to change event polling mechanism on the fly to prevent hogging of cpu resources
			case SDL_WINDOWEVENT_FOCUS_LOST:
				hasFocus = false; break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				hasFocus = true; break;
			// Handle resizing
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				window.updateDstRect(); break;
			}
		};
		
		using namespace Input;
		axisInputManager.setKeyMapping(Axis::Main_X, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, -1.0, 0.0, 1.0);
		axisInputManager.setKeyMapping(Axis::Main_Y, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, -1.0, 0.0, 1.0);
		axisInputManager.setKeyMapping(Radial::Main, SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D);
		axisInputManager.setKeyMapping(Axis::Aux_0, SDL_SCANCODE_LCTRL, 0.0, 1.0);
				
		for(int i = 0; i < ReSDL::Joystick::numJoysticks(); ++i)
		{
			auto axisMapping = [](Sint16 value) -> double {
				// scales to [-1.0,1.0]
				return (double)value / std::numeric_limits<Sint16>::max();
			};
			auto triggerMapping = [](Sint16 value) -> double {
				// scales [0... SInt max] to [0.0,1.0] 
				return (double)(value) / std::numeric_limits<Sint16>::max();
			};

			auto radialJoyMapping = [](Sint16 x, Sint16 y) -> Vec2d {
				return { (double)x / std::numeric_limits<Sint16>::max(), (double)y / std::numeric_limits<Sint16>::max() };
			};

			if(ReSDL::GameController::isGameController(i))
			{
				auto gc = std::make_shared<ReSDL::GameController>(i);
				axisInputManager.setGameControllerMapping(gc,
					SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX,
					Axis::Main_X,
					axisMapping);
				axisInputManager.setGameControllerMapping(gc,
					SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY,
					Axis::Main_Y,
					axisMapping);
				axisInputManager.setGameControllerMapping(gc,
					SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
					Axis::Aux_0,
					triggerMapping);
				axisInputManager.setGameControllerMapping(gc,
					SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT,
					Axis::Aux_1,
					triggerMapping);

				axisInputManager.setGameControllerMapping(gc,
					SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX,
					SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY,
					Radial::Main,
					radialJoyMapping);
				axisInputManager.setGameControllerMapping(gc,
					SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX,
					SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY,
					Radial::Secondary,
					radialJoyMapping);

				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A, Button::A);
				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B, Button::B);
				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X, Button::X);
				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y, Button::Y);
				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP, Button::DPad_Up);
				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN, Button::DPad_Down);
				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT, Button::DPad_Left);
				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT, Button::DPad_Right);
				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER, Button::Shoulder_Left);
				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, Button::Shoulder_Right);
				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSTICK, Button::Stick_Left);
				axisInputManager.setGameControllerMapping(gc, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK, Button::Stick_Right);

				axisInputManager.setGameControllerMapping(gc, 
					SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP,
					SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN,
					SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT,
					SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT, 
					Radial::Tertiary);

				axisInputManager.setButtonHandler(Button::B, [&device, &ad](Button b, ButtonState s) {
					switch(s) {
						case ButtonState::Push:
							{
								// lock audio
								device->lock();
								ad.volume = 1.0;
								device->unlock();
								// unlock audio
								// if (device->getQueuedAudioSize() <= data.size()) {
								// 	device->queueAudio(data);
								// }
							}
							break;
						case ButtonState::Release:
								ad.volume = 0.0;
							//device->clearQueuedAudio();
							break;
					} 
				});
			}
			else
			{
				auto joystick = std::make_shared<ReSDL::Joystick>(i);
				axisInputManager.setJoystickMapping(joystick, 0, Axis::Main_X, axisMapping);
				axisInputManager.setJoystickMapping(joystick, 1, Axis::Main_Y, axisMapping);
			}
		}
		
		Ticks frameTicks;
		std::chrono::microseconds accumulatedFrameTimes{};
		int frameCount = 0;

		// Main game loop starts here..
		while(!isDone)
		{
			// Event handling
			if(!hasFocus)
			{
				eventManager.waitAndHandle();
			}
			else
			{
				eventManager.pollAndHandle();
			}
			axisInputManager.handleAndSubmitEvents();
			
			// frame time calculation
			const auto ticks = frameTicks.elapsedMs();
			accumulatedFrameTimes += ticks;
			frameTicks = Ticks();
			
			for(auto &updateable : m_Updateables)
			{
				updateable->update(ticks);
			}
			
			window.prepareFrame();
			for(auto &renderable : m_Renderables)
			{
				renderable->render(*window.renderer());
			}
			window.finalizeFrame();
		
			frameCount++;
			
			if(frameCount % 100 == 0)
			{
				std::cout << 1000.0 / (accumulatedFrameTimes.count() / frameCount) << " fps" << std::endl;
				accumulatedFrameTimes = std::chrono::microseconds{};
				frameCount = 0;
			}
		}
	}
	
	
}
