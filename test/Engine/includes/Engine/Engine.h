//
//  Engine.h
//  libsdlTest
//
//  Created by Andreas Stahl on 23.02.15.
//  Copyright (c) 2015 Andreas. All rights reserved.
//

#pragma once

#include <vector>
#include <array>
#include <random>
#include <map>
#include <set>
#include <iostream>
#include <functional>
#include <chrono>

#include "ReSDL/ReSDL.h"

#include "Input/AxisInputManager.h"
#include "Input/EventManager.h"
#include "Utilities.h"

namespace Engine {


	
class RttRendererWindow {
	
	const int m_TargetWidth;
	const int m_TargetHeight;
	const float m_AspectRatio;

	SDL_Rect m_dstRect;
	
	std::shared_ptr<ReSDL::Window> m_Window;
	std::shared_ptr<ReSDL::Renderer> m_Renderer;
	ReSDL::Texture m_TargetTexture;
	
public:
	RttRendererWindow(int targetWidth, int targetHeight, float pixelAspectRatio);
	void prepareFrame();
	void finalizeFrame();
	void updateDstRect();
	
	std::shared_ptr<ReSDL::Renderer> renderer();
};

class IUpdatable {
public:
		virtual void update(std::chrono::microseconds) = 0;
};
	
class IRenderable {
public:
		virtual int order() { return 0; }
		virtual void render(ReSDL::Renderer&) = 0;
};
	
	
struct Engine {
	ReSDL::SDL sdl;
	
	RttRendererWindow window;
	Input::AxisInputManager axisInputManager;
	Input::EventManager eventManager;
	
	std::vector<std::shared_ptr<IUpdatable>> m_Updateables;
	std::vector<std::shared_ptr<IRenderable>> m_Renderables;
	
	long frameCount;
	
	Engine(int width, int height, float pixelAspectRatio);

	void start();
	void addRenderable(std::shared_ptr<IRenderable> renderable);
	void addUpdateable(std::shared_ptr<IUpdatable> updateable);
};


template<typename MassType, typename SpatialType, typename TimeType, size_t Dimensions>
class PhysicsPoint
{
public:
	using ImpulseType = decltype(MassType() * SpatialType());
	using VelocityType = decltype(TimeType().count() * SpatialType());
	using PositionVector = Vec<SpatialType, Dimensions>;
	using VelocityVector = Vec<VelocityType, Dimensions>;
	using ImpulseVector = Vec<ImpulseType, Dimensions>;
	
	
	PhysicsPoint(MassType mass, const PositionVector &position, const VelocityVector &velocity)
	: m_mass(mass)
	, m_position(position)
	, m_velocity(velocity)
	{
	}
	
	template<typename DurationType>
	PhysicsPoint advance(Vec2d impulse, DurationType duration) const
	{
		const auto time = std::chrono::duration_cast<TimeType>(duration);
		const auto changeInVelocity = impulse / m_mass;
		const auto velocity = m_velocity + changeInVelocity;
		const auto changeInPosition = (m_velocity * time.count());
		return PhysicsPoint(m_mass, m_position + changeInPosition, velocity);
	}
	
	ImpulseVector getImpulse() const
	{
		return m_velocity * m_mass;
	}
	
	PositionVector getPosition() const
	{
		return m_position;
	}
	
	MassType getMass() const
	{
		return m_mass;
	}
	
private:
	MassType m_mass;
	PositionVector m_position;
	VelocityVector m_velocity;
};
	
using PhysicsPoint2d = PhysicsPoint<double, double, std::chrono::milliseconds, 2>;

	
class PointDistribution {
public:
	PointDistribution(int count)
	{
		std::random_device generator;
		std::uniform_real_distribution<float> distribution(0,1.0);
		
		for(int i = count; i > 0; i--) {
			points.push_back({distribution(generator),distribution(generator)});
		}
	}
	
	std::vector<SDL_Point> mapToWindow(int scale, SDL_Point origin, SDL_Point windowSize) const
	{
		std::vector<SDL_Point> result;
		SDL_Point extent{scale, scale};
		for(const auto& p : points ) {
			// scale up point
			SDL_Point point = {static_cast<int>(p.first * scale), static_cast<int>(p.second * scale)};
			// get normalized and positive window extends
			SDL_Point originNorm = origin % extent;
			originNorm.x += (originNorm.x < 0 ? scale : 0);
			originNorm.y += (originNorm.y < 0 ? scale : 0);
			// transform point to window
			point = point - originNorm;
			point.x += (point.x < 0 ? scale : 0);
			point.y += (point.y < 0 ? scale : 0);
			if(point < windowSize)
				result.push_back(point);
		}
		return result;
	}
	
private:
	std::vector<std::pair<float, float>> points;
};


class SpriteSheet {
public:
	SpriteSheet(ReSDL::Texture&& texture, std::vector<SDL_Rect> rectangles = std::vector<SDL_Rect>())
	: texture(std::move(texture))
	, rectangles(rectangles)
	{
	}
	
	SpriteSheet(ReSDL::Texture&& texture, size_t columns, size_t rows)
	: texture(std::move(texture))
	{
		rows = std::max<size_t>(rows, 1);
		columns = std::max<size_t>(columns, 1);
		ReSDL::Size size{ texture.size };
		size.width /= columns;
		size.height /= rows;
		for(int iRow = 0; iRow < rows; ++iRow) {
			for(int iColumn = 0; iColumn < columns; ++iColumn) {
				rectangles.push_back({iColumn * size.width, iRow * size.height, size.width, size.height});
			}
		}
	}
	
	size_t getNumRects() const {
		return rectangles.size();
	}
	
	const SDL_Rect *getRect(size_t index = 0) const {
		if(getNumRects() == 0) {
			return nullptr;
		}
		return &rectangles[index % getNumRects()];
	}
	
private:
	ReSDL::Texture texture;
	std::vector<SDL_Rect> rectangles;
};
	
	struct SpriteAnimationFrame
	{
		int index;
		SDL_Point offset;
		std::chrono::milliseconds duration;
	};
	
	class SpriteAnimation
	{
	public:
		SpriteAnimation(const std::shared_ptr<SpriteSheet> &sheet, const std::vector<SpriteAnimationFrame> &frames)
		: m_sheet(sheet)
		, m_frames(frames)
		{
			
		}
		
	private:
		std::shared_ptr<SpriteSheet> m_sheet;
		std::vector<SpriteAnimationFrame> m_frames;
	};




class Ticks {
public:
	Ticks() : baseline(SDL_GetTicks()) {}
	
	std::chrono::microseconds elapsedMs() const {
		return std::chrono::microseconds(SDL_GetTicks() - baseline);
	}
private:
	Uint32 baseline;
};

}