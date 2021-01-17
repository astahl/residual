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
	
	SDL_Rect m_Rectangle;
	Vec2d m_Speed;
	Vec2d m_Position;
public:
	Sprite(int w, int h, int x = 0, int y = 0)
	: m_Rectangle({x, y, w, h})
	{
		
	}
	
	void render(ReSDL::Renderer& renderer)
	{
		SDL_Rect rect{static_cast<int>(m_Rectangle.x + m_Position[0]), static_cast<int>(m_Rectangle.y + m_Position[1]), m_Rectangle.w, m_Rectangle.h};
		renderer.setDrawColor(ReSDL::Color::White);
		renderer.drawRect(&rect);
	}
	
	void update(std::chrono::microseconds deltaT)
	{
		m_Position = m_Position + m_Speed * deltaT.count();
	}
	
	void setSpeed(const Vec2d &speed)
	{
		m_Speed = speed;
	}
	
	void setSpeedX(double x)
	{
		m_Speed[0] = x;
	}
	
	void setSpeedY(double y)
	{
		m_Speed[1] = y;
	}
};

class Game {
	const int m_TargetWidth;
	const int m_TargetHeight;
	Engine::Engine m_Engine;
	
	int m_I;
public:
	
	Game()
		: m_TargetWidth(320)
		, m_TargetHeight(200)
		, m_Engine{ m_TargetWidth, m_TargetHeight }
	, m_I(0)
	{
		using namespace Engine::Input;
		
		std::shared_ptr<Sprite> playerSprite = std::make_shared<Sprite>(10, 10);
		m_Engine.axisInputManager()->setAxisOffValue(Axis::Main_X, 0);
		m_Engine.axisInputManager()->setAxisHandler(Axis::Main_X, [playerSprite](const Axis&, const double& value) {
			playerSprite->setSpeedX(value / 10);
		});
		m_Engine.axisInputManager()->setAxisOffValue(Axis::Main_Y, 0);
		m_Engine.axisInputManager()->setAxisHandler(Axis::Main_Y, [playerSprite](const Axis&, const double& value) {
			playerSprite->setSpeedY(value / 10);
		});
		
		Vec<Uint8, 4> color({255, 0, 255, 0});
		m_Engine.addRenderable(std::make_shared<ClearScreen>(color));
		m_Engine.addRenderable(playerSprite);
		m_Engine.addUpdateable(playerSprite);
	}
	
	~Game() {
	}
	
	int run()
	{
		m_Engine.start();
		return 0;
	}
	
	void render(ReSDL::Renderer& renderer)
	{
		renderer.setDrawColor(ReSDL::Color::Cyan);
		renderer.clear();
		renderer.setDrawColor(ReSDL::Color::Magenta);
		renderer.drawLine(m_I, 0, m_TargetWidth - m_I, m_TargetHeight);
		renderer.drawLine(m_TargetWidth - m_I, 0, m_I, m_TargetHeight);
		
	}
	
	void update(std::chrono::microseconds ticks)
	{
		m_I++;
		if(m_I > m_TargetWidth) m_I = 0;
	}
	
};


int main()
{
	Game game;
	return game.run();
}