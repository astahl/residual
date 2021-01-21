#include <Engine/Engine.h>


namespace Engine {
	
	
	RttRendererWindow::RttRendererWindow(int targetWidth, int targetHeight, float pixelAspectRatio)
	: m_TargetWidth(targetWidth)
	, m_TargetHeight(targetHeight)
	, m_AspectRatio(pixelAspectRatio > 0 ? 1 / pixelAspectRatio : static_cast<float>(targetWidth) / static_cast<float>(targetHeight))
		, m_Window{ std::make_shared<ReSDL::Window>("Puup",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			2 * m_TargetWidth,
			2 * m_TargetHeight,
			SDL_WINDOW_OPENGL
			| SDL_WINDOW_ALLOW_HIGHDPI
			| SDL_WINDOW_RESIZABLE) }
		, m_Renderer{ std::make_shared<ReSDL::Renderer>(*m_Window,
			0,
			SDL_RENDERER_ACCELERATED
			| SDL_RENDERER_PRESENTVSYNC
			| SDL_RENDERER_TARGETTEXTURE) }
	{
		m_TargetTexture = ReSDL::Texture{ *m_Renderer, 0, SDL_TEXTUREACCESS_TARGET, m_TargetWidth, m_TargetHeight };	
		updateDstRect();
	}
	
	void RttRendererWindow::prepareFrame()
	{
		m_Renderer->setRenderTarget(m_TargetTexture);
	}
	
	void RttRendererWindow::finalizeFrame()
	{
		// render target texture to window
		m_Renderer->setRenderTarget(nullptr);
		m_Renderer->setDrawColor(ReSDL::Color::Black);
		m_Renderer->clear();
		m_Renderer->copy(m_TargetTexture, nullptr, &m_dstRect);
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
	: m_sdl(std::make_unique<ReSDL::SDL>())
	, m_Window(std::make_unique<RttRendererWindow>(width, height, pixelAspectRatio))
	, m_AxisInputManager(std::make_shared<Input::AxisInputManager>())
	, m_EventManager(std::make_shared<Input::EventManager>())
	, m_FrameCount(0)
	{
		
	}
	
	Engine::~Engine()
	{}
	
	
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
	
	std::shared_ptr<Input::AxisInputManager> Engine::axisInputManager()
	{
		return m_AxisInputManager;
	}
	
	std::shared_ptr<Input::EventManager> Engine::eventManager()
	{
		return m_EventManager;
	}
	
	void Engine::start()
	{
		bool isDone = false;
		// Handle quit events by setting the loop exit variable
		m_EventManager->setHandler(SDL_QUIT, [&isDone](const SDL_Event&) { isDone = true; });
		
		bool hasFocus = false;
		m_EventManager->setHandler(SDL_WINDOWEVENT, [&hasFocus, this](const SDL_Event& e) {
			switch (e.window.event)
			{
			// Handle focussing of window to change event polling mechanism on the fly to prevent hogging of cpu resources
			case SDL_WINDOWEVENT_FOCUS_LOST:
				hasFocus = false; break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				hasFocus = true; break;
			// Handle resizing
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				m_Window->updateDstRect(); break;
			}
		});
		
		using namespace Input;
		m_AxisInputManager->setKeyMapping(SDL_SCANCODE_LEFT, Axis::Main_X, -1.0);
		m_AxisInputManager->setKeyMapping(SDL_SCANCODE_RIGHT, Axis::Main_X, 1.0);
		m_AxisInputManager->setKeyMapping(SDL_SCANCODE_UP, Axis::Main_Y, -1.0);
		m_AxisInputManager->setKeyMapping(SDL_SCANCODE_DOWN, Axis::Main_Y, 1.0);
		m_AxisInputManager->setKeyMapping(SDL_SCANCODE_A, Axis::Main_X, -1.0);
		m_AxisInputManager->setKeyMapping(SDL_SCANCODE_D, Axis::Main_X, 1.0);
		m_AxisInputManager->setKeyMapping(SDL_SCANCODE_W, Axis::Main_Y, -1.0);
		m_AxisInputManager->setKeyMapping(SDL_SCANCODE_S, Axis::Main_Y, 1.0);
		
		for(int i = 0; i < ReSDL::Joystick::numJoysticks(); ++i)
		{
			auto joyMapping = [](Sint16 value) -> double {
				// scales to [-1.0,1.0]
				return static_cast<double>(value) / std::numeric_limits<Sint16>::max();
			};
			auto joyMapping2 = [](Sint16 value) -> double {
				// scales to [0.0,1.0]
				return (static_cast<double>(value) - std::numeric_limits<Sint16>::min()) / std::numeric_limits<int16_t>::max();
			};
			if(ReSDL::GameController::isGameController(i))
			{
				auto gc = std::make_shared<ReSDL::GameController>(i);
				m_AxisInputManager->setGameControllerMapping(gc,
																										 SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX,
																										 Axis::Main_X,
																										 joyMapping);
				m_AxisInputManager->setGameControllerMapping(gc,
																										 SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY,
																										 Axis::Main_Y,
																										 joyMapping);
				m_AxisInputManager->setGameControllerMapping(gc,
																										 SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
																										 Axis::Aux_0,
																										 joyMapping2);
			}
			else
			{
				auto joystick = std::make_shared<ReSDL::Joystick>(i);
				m_AxisInputManager->setJoystickMapping(joystick, 0, Axis::Main_X, joyMapping);
				m_AxisInputManager->setJoystickMapping(joystick, 1, Axis::Main_Y, joyMapping);
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
				m_EventManager->waitAndHandle();
			}
			else
			{
				m_EventManager->pollAndHandle();
			}
			m_AxisInputManager->handleAndSubmitEvents();
			
			// frame time calculation
			const auto ticks = frameTicks.elapsedMs();
			accumulatedFrameTimes += ticks;
			frameTicks = Ticks();
			
			for(auto &updateable : m_Updateables)
			{
				updateable->update(ticks);
			}
			
			m_Window->prepareFrame();
			for(auto &renderable : m_Renderables)
			{
				renderable->render(*m_Window->renderer());
			}
			m_Window->finalizeFrame();

			m_FrameCount++;
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
