//
//  main.cpp
//  libsdlTest
//
//  Created by Andreas Stahl on 01.02.15.
//  Copyright (c) 2015 Andreas. All rights reserved.
//

#include <iostream>

#include "ReSDL/ReSDL.h"

//#include <SDL_mixer.h>

#include "Engine/Engine.h"

int main(int argc, const char * argv[])
{
	using namespace Engine::Input;
	using namespace Engine;
	
	
	Vec2d a({3.1,2.0});
	Vec2d b({-3.0, 2.4});
	std::cout << a << (a+b).length();
	
	auto sdl = ReSDL::SDL();
	const int windowWidth = 320;
	const int windowHeight = 200;
	const float scale = 1;
	const int width = windowWidth * scale;
	const int height = windowHeight * scale;
	
	ReSDL::Window window("Gänsefluch",
														SDL_WINDOWPOS_CENTERED,
														SDL_WINDOWPOS_CENTERED,
														windowWidth,
														windowHeight,
														SDL_WINDOW_SHOWN
										 //| SDL_WINDOW_FULLSCREEN_DESKTOP
														| SDL_WINDOW_ALLOW_HIGHDPI
														);
	ReSDL::Renderer renderer(window.raw(), -1,
												 SDL_RENDERER_ACCELERATED
												 | SDL_RENDERER_PRESENTVSYNC
												 );
	renderer.setLogicalSize(width, height);
	renderer.setDrawBlendMode(SDL_BLENDMODE_BLEND);
	PointDistribution dist(300);
	PointDistribution dist2(100);
	PointDistribution dist3(50);
	PointDistribution dist4(50);

	uint64_t frame = 0;
	
	Vec2d position;
	Vec2d velocity;
	
	Vec2d impulse;
	PhysicsPoint2d point(1.0, Vec2d(), Vec2d({1.0,0.0}));
	
	ReSDL::Image image;
	ReSDL::Surface surface(IMG_Load("goose2.png"));
	auto tex = std::make_shared<ReSDL::Texture>(renderer.get(), surface.get());
	SDL_Rect goose_dest{(int)position[0], (int)position[1], surface.getWidth() / 2, surface.getHeight() / 2};
	SpriteSheet sprite(tex, 1,1);
	
	
	ReSDL::Surface cloud(IMG_Load("puup.png"));
	auto tex2 = std::make_shared<ReSDL::Texture>(renderer.get(), cloud.get());
	SpriteSheet spriteCloud(tex2, 4,1);
	
	Mix_Init(MIX_INIT_OGG);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
	Mix_Music *sound = Mix_LoadMUS("music.mid");
	Mix_PlayMusic(sound, -1);
	
	EventManager events;
	AxisInputManager aim;
	bool isDone = false;
	
	events.setHandler(SDL_QUIT, [&isDone](const SDL_Event&) {	isDone = true; });
	
	aim.setKeyMapping(SDL_SCANCODE_LEFT, Axis::Main_X, -1.0);
	aim.setKeyMapping(SDL_SCANCODE_RIGHT, Axis::Main_X, 1.0);
	aim.setKeyMapping(SDL_SCANCODE_UP, Axis::Main_Y, -1.0);
	aim.setKeyMapping(SDL_SCANCODE_DOWN, Axis::Main_Y, 1.0);
	aim.setKeyMapping(SDL_SCANCODE_A, Axis::Main_X, -1.0);
	aim.setKeyMapping(SDL_SCANCODE_D, Axis::Main_X, 1.0);
	aim.setKeyMapping(SDL_SCANCODE_W, Axis::Main_Y, -1.0);
	aim.setKeyMapping(SDL_SCANCODE_S, Axis::Main_Y, 1.0);
	aim.setKeyMapping(SDL_SCANCODE_LSHIFT, Axis::Aux_0, 1.0);
	aim.setKeyMapping(SDL_SCANCODE_RSHIFT, Axis::Aux_0, 1.0);

	double mod = 0.0;
	aim.setAxisOffValue(Axis::Aux_0, mod);
	aim.setAxisHandler(Axis::Aux_0,
										 [&mod](const Axis&, const double &value)
										 {
											 mod = 5 * value;
										 });
	aim.setAxisHandler(Axis::Main_X,
										 [&mod, &velocity](const Axis&, const double &value)
										 {
											 velocity[0] += value + mod * value;
										 });
	
	aim.setAxisHandler(Axis::Main_Y,
										 [&mod, &velocity](const Axis&, const double &value)
										 {
											 velocity[1] += value + mod * value;
										 });
	
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
			aim.setGameControllerMapping(gc,
																	 SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX,
																	 Axis::Main_X,
																	 joyMapping);
			aim.setGameControllerMapping(gc,
																	 SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY,
																	 Axis::Main_Y,
																	 joyMapping);
			aim.setGameControllerMapping(gc,
																	 SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
																	 Axis::Aux_0,
																	 joyMapping2);
		}
		else
		{
			auto joystick = std::make_shared<ReSDL::Joystick>(i);
			aim.setJoystickMapping(joystick, 0, Axis::Main_X, joyMapping);
			aim.setJoystickMapping(joystick, 1, Axis::Main_Y, joyMapping);
		}
	}
	Ticks frameTicks;
	
	while(!isDone)
	{
		// Event handling
		events.pollAndHandle();
		aim.handleAndSubmitEvents();
		// frame time calculation
		
		const auto ticks = frameTicks.elapsedMs();
		frameTicks = Ticks();
		
		point = point.advance(impulse, ticks);
		
		std::multimap<int, std::function<void(const ReSDL::Renderer &)>> renderJobs;
		
		// prepare frame
		renderer.setDrawColor(ReSDL::Color::LightBlue);
		renderer.clear();
		// stars
//		auto stars = dist.mapToWindow(width, {(int)(fx * 0.8), (int)(fy * 0.8)}, {width, height});
//		if(!stars.empty()) {
//			renderer.setDrawColor(ReSDL::Color::DarkGrey);
//			renderer.drawPoints(&stars[0], stars.size());
//		}
//		stars = dist2.mapToWindow(width, {(int)fx, (int)fy}, {width, height});
//		if(!stars.empty()) {
//			renderer.setDrawColor(ReSDL::Color::Grey);
//			renderer.drawPoints(&stars[0], stars.size());
//		}
//		stars = dist3.mapToWindow(width * 2, {(int)(fx * 2), (int)(fy * 2)}, {width, height});
//		if(!stars.empty()) {
//			renderer.setDrawColor(ReSDL::Color::LightGrey);
//			renderer.drawPoints(&stars[0], stars.size());
//		}
//		stars = dist4.mapToWindow(width * 3, {(int)(fx * 3), (int)(fy * 3)}, {width, height});
//		if(!stars.empty()) {
//			renderer.setDrawColor(ReSDL::Color::White);
//			renderer.drawPoints(&stars[0], stars.size());
//		}
		for(auto &p : dist.mapToWindow(width * 2, (position * 2).toSDLPoint(), {width, height}))
		{
			SDL_Rect dest = ReSDL::RectMoveTo(*spriteCloud.getRect(), p);
			//renderer.copy(spriteCloud.getTexture()->get(), spriteCloud.getRect(), &dest);
			renderJobs.insert({0, [=](const ReSDL::Renderer& r) { r.copy(spriteCloud.getTexture()->get(),
																																	 spriteCloud.getRect(),
																																	 &dest);}});
			
		}
		for(auto &p : dist.mapToWindow(width * 3, (position * 3).toSDLPoint(), {width, height}))
		{
			SDL_Rect dest = ReSDL::RectMoveTo(*spriteCloud.getRect(), p);
			//renderer.copy(spriteCloud.getTexture()->get(), spriteCloud.getRect(), &dest);
			renderJobs.insert({2, [=](const ReSDL::Renderer& r) { r.copy(spriteCloud.getTexture()->get(),
																																	 spriteCloud.getRect(),
																																	 &dest);}});
			
		}
		
		
		// goose
		const SDL_Rect *srcRect = sprite.getRect(frame / 25);
		SDL_Rect destRect {
			static_cast<int>((width - goose_dest.w) / 2 + velocity[0]),
			static_cast<int>((height - goose_dest.h) / 2  + velocity[1]),
			goose_dest.w,
			goose_dest.h
		};
		double angle = atan2(velocity[1], velocity[0]) * 180 / 3.141;
		renderJobs.insert({1, [=](const ReSDL::Renderer& r) {
			r.copyEx(sprite.getTexture()->get(), srcRect, &destRect, angle, nullptr, SDL_FLIP_NONE);
			}});
		
		for(const auto& job : renderJobs)
		{
			job.second(renderer);
		}
		
		// end frame
		renderer.present();
		frame++;
		position = position + velocity * 0.1;
		velocity = velocity * 0.98;
		if(fabs(velocity[0]) < 0.1) velocity[0] = 0;
		if(fabs(velocity[1]) < 0.1) velocity[1] = 0;
	}
	Mix_Quit();
	return 0;
}
