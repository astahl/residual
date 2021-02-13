#include "Engine/Engine.h"
#include <SDL_opengl.h>


class DrawColorKeeper {
	std::shared_ptr<ReSDL::Renderer> m_Renderer;
	Uint8 m_R;
	Uint8 m_G;
	Uint8 m_B;
	Uint8 m_A;
public:
	DrawColorKeeper(std::shared_ptr<ReSDL::Renderer> renderer)
	: m_Renderer(renderer)
	{
		m_Renderer->getDrawColor(&m_R, &m_G, &m_B, &m_A);
	}
	
	~DrawColorKeeper()
	{
		m_Renderer->setDrawColor(m_R, m_G, m_B, m_A);
	}
};

class ClearScreen : public Engine::IRenderable
{
	Vec<Uint8, 4> color;
public:
	ClearScreen(Vec<Uint8, 4> color)
	: color(color) {}
	
	int order() {
		return -1;
	}
	
	void render(ReSDL::Renderer& renderer)
	{
		renderer.setDrawColor(color[0], color[1], color[2], color[3]);
		renderer.clear();
	}
};

struct PhysicsPoint : public Engine::IUpdatable {
	
	Vec2d m_Speed;
	Vec2d m_Position;
	Vec2d m_Impulse;
	
};

class Sprite : public Engine::IRenderable, public Engine::IUpdatable
{
	
	Vec2d m_Position;
	SDL_Rect m_Rectangle;
	Vec2d speed;
public:
	Vec2d acceleration;
	double friction;

	Sprite(int w, int h, int x = 0, int y = 0)
	: m_Rectangle({x, y, w, h})
		, friction {0.1}
	{
		
	}
	
	void render(ReSDL::Renderer& renderer)
	{
		SDL_Rect rect{static_cast<int>(m_Rectangle.x + m_Position[0]), static_cast<int>(m_Rectangle.y + m_Position[1]), m_Rectangle.w, m_Rectangle.h};
		renderer.setDrawColor(ReSDL::Color::Green);
		if (speed.x() == 0 && speed.y() == 0)
			renderer.fillRect(rect);
		renderer.setDrawColor(ReSDL::Color::White);
		renderer.drawRect(rect);
	}
	
	void update(std::chrono::microseconds deltaT)
	{
		speed = speed + (acceleration * deltaT.count());
		speed = speed * (1 - friction);
		if (speed.lengthSquared() < 0.0001) 
			speed = Vec2d{};
		m_Position = m_Position + speed * deltaT.count();
	}
};

struct AxisDebug : public Engine::IRenderable
{
	Vec2i position;
	Vec2i size;
	double value;
	double min;
	double max;
	
	AxisDebug(Vec2i position, Vec2i size, double min = -1.0, double max = 1.0)
		: position{position}
		, size{size}
		, value{0}
		, min{min}
		, max{max}
	{
	}

	void render(ReSDL::Renderer& renderer)
	{
		SDL_Rect rect{ position.x(), position.y(), size.x() + 1, size.y() + 1 };
		renderer.setDrawColor(ReSDL::Color::White);
		renderer.drawRect(rect);
		const double range = max - min;
		const int valuePos = (value - min) / range * (double)size.x();
		renderer.drawLine(position.x() + valuePos, position.y() - 1, position.x() + valuePos, position.y() + size.y() + 1);
	}
};

struct RadialDebug : public Engine::IRenderable
{
	Vec2i position;
	Vec2i size;
	Vec2d value;
	double min;
	double max;

	RadialDebug(Vec2i position, Vec2i size, double min = -1.0, double max = 1.0)
		: position{ position }
		, size{ size }
		, value{}
		, min{ min }
		, max{ max }
	{
	}

	void render(ReSDL::Renderer& renderer)
	{
		SDL_Rect rect{ position.x(), position.y(), size.x() + 1, size.y() + 1 };
		Vec2d center = size * 0.5 + position;
		renderer.setDrawColor(ReSDL::Color::White);
		renderer.drawRect(rect);
		const double range = max - min;
		const int valuePosX = (value.x() - min) / range * (double)size.x();
		const int valuePosY = (value.y() - min) / range * (double)size.y();
		renderer.drawLine(center.x(), center.y(), position.x() + valuePosX, position.y() + valuePosY);
	}
};

struct ButtonDebug : public Engine::IRenderable
{
	Vec2i position;
	Vec2i size;
	Engine::Input::ButtonState value;

	ButtonDebug(Vec2i position, Vec2i size)
		: position{ position }
		, size{ size }
		, value{Engine::Input::ButtonState::Off}
	{
	}

	void render(ReSDL::Renderer& renderer)
	{
		SDL_Rect rect{ position.x(), position.y(), size.x(), size.y() };
		renderer.setDrawColor(value == Engine::Input::ButtonState::Off 
			? ReSDL::Color::Black 
			: value == Engine::Input::ButtonState::Hold 
				? ReSDL::Color::Green
				: ReSDL::Color::Magenta);
		renderer.fillRect(rect);
		renderer.setDrawColor(value == Engine::Input::ButtonState::Push ? ReSDL::Color::Black : value == Engine::Input::ButtonState::Release ? ReSDL::Color::Black : ReSDL::Color::White);
		renderer.drawRect(rect);
	}
};


class Game {
	Engine::Engine m_Engine;
public:
	
	Game()
		: m_Engine{ 640, 480, 2.4/3.2 }
	{
		using namespace Engine::Input;
		
		std::shared_ptr<Sprite> playerSprite = std::make_shared<Sprite>(10, 10);
		std::shared_ptr<AxisDebug> xDebug = std::make_shared<AxisDebug>(Vec2i{ 100, 100 }, Vec2i{ 100, 10 });
		std::shared_ptr<AxisDebug> triggerDebug = std::make_shared<AxisDebug>(Vec2i{ 100, 120 }, Vec2i{ 100, 10 }, 0.0);
		std::shared_ptr<RadialDebug> stickrDebug = std::make_shared<RadialDebug>(Vec2i{ 100, 10 }, Vec2i{ 80, 80 });
		std::shared_ptr<ButtonDebug> buttonDebug = std::make_shared<ButtonDebug>(Vec2i{ 50, 50 }, Vec2i{ 10, 10 });
		m_Engine.axisInputManager.setAxisHandler(Axis::Main_X, [playerSprite, xDebug](const Axis&, const double& value) {
			playerSprite->acceleration.x() = (value / 1000);
			xDebug->value = value;
		});
		m_Engine.axisInputManager.setAxisHandler(Axis::Main_Y, [playerSprite](const Axis&, const double& value) {
			playerSprite->acceleration.y()= (value / 1000);
		});

		m_Engine.axisInputManager.setRadialHandler(Radial::Main, [playerSprite, stickrDebug](const Radial&, const Vec2d& value) {
			playerSprite->acceleration = value / 1000;
			stickrDebug->value = value;
		});

		m_Engine.axisInputManager.setAxisHandler(Axis::Aux_0, [playerSprite, triggerDebug](const Axis&, const double& value) {
			playerSprite->friction = 0.1 * (1.0 - value);
			triggerDebug->value = value;
		});

		m_Engine.axisInputManager.setButtonHandler(Button::A, [buttonDebug](const Button& button, const ButtonState& state) {
			buttonDebug->value = state;
		});

		Vec<Uint8, 4> color({255, 0, 255, 0});
		m_Engine.addRenderable(std::make_shared<ClearScreen>(color));

		m_Engine.addRenderable(playerSprite);
		m_Engine.addRenderable(xDebug);
		m_Engine.addRenderable(triggerDebug);
		m_Engine.addRenderable(stickrDebug);
		m_Engine.addRenderable(buttonDebug);

		m_Engine.addUpdateable(playerSprite);
	}
	
	~Game() {
	}
	
	int run()
	{
		m_Engine.start();
		return 0;
	}
	
};


int main()
{
	Game game;
	return game.run();
}