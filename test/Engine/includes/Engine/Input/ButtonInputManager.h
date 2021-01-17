//
//  AxisInputManager.h
//  libsdlTest
//
//  Created by Andreas Stahl on 23.02.15.
//  Copyright (c) 2015 Andreas. All rights reserved.
//

#pragma once
#include <functional>
#include <map>
#include <memory>

namespace Engine {
	namespace Input {
		using ButtonId = int;
		using ButtonInputHandlerFunc = std::function<void(const ButtonId&, bool)>;
		
		class ButtonInputManager
		{
			std::map<Uint8, ButtonId> keyMappings;
			std::map<std::shared_ptr<ReSDL::Joystick>, std::map<int, ButtonId>> joystickMappings;
			std::map<std::shared_ptr<ReSDL::GameController>, std::map<SDL_GameControllerButton, ButtonId>> controllerMappings;
			std::map<ButtonId, ButtonInputHandlerFunc> handlers;
			
			void handleKeyState(std::map<const ButtonId&, bool> &values) const
			{
				const Uint8 *state = SDL_GetKeyboardState(NULL);
				
				for(const auto &keyMapping : keyMappings)
				{
					auto button = std::get<0>(keyMapping.second);
					if(handlers.count(button))
					{
						ButtonInputHandlerFunc handler = handlers.at(axis);
						if(state[keyMapping.first])
						{
							const double &value = std::get<1>(keyMapping.second);
							values[axis] = value;
						}
					}
				}
			}
			
			void handleJoysticks(std::map<Axis, double> &values) const
			{
				for(auto &joystickMapping : joysticks)
				{
					auto &joystick = joystickMapping.first;
					for(auto &joyAxisMapping : joystickMapping.second)
					{
						Sint16 axisValue = joystick->getAxis(joyAxisMapping.first);
						const Axis &axis = std::get<1>(joyAxisMapping.second);
						double value = std::get<0>(joyAxisMapping.second)(axisValue);
						if(fabs(value) > 0.05)
						{
							values[axis] = value;
						}
					}
				}
			}
			
			void handleGameControllers(std::map<Axis, double> &values) const
			{
				for(auto &gameControllerMapping : controllerMapping)
				{
					auto &gameController = gameControllerMapping.first;
					for(auto &gcAxisMapping : gameControllerMapping.second)
					{
						Sint16 axisValue = gameController->getAxis(gcAxisMapping.first);
						const Axis &axis = std::get<1>(gcAxisMapping.second);
						double value = std::get<0>(gcAxisMapping.second)(axisValue);
						if(fabs(value) > 0.05)
						{
							values[axis] = value;
						}
					}
				}
			}
			
			void submitValues(const std::map<Axis, double> &values) const
			{
				std::set<Axis> handledAxes;
				for(auto &axisValues : values) {
					const Axis &axis = axisValues.first;
					if(handlers.count(axis))
					{
						AxisInputHandlerFunc handler = handlers.at(axis);
						handler(axis, axisValues.second);
						handledAxes.insert(axis);
					}
				}
				for(const auto &offValueMapping : offValues) {
					const Axis &axis = offValueMapping.first;
					if(handlers.count(axis) && handledAxes.count(axis) == 0) {
						AxisInputHandlerFunc handler = handlers.at(axis);
						handler(axis, offValueMapping.second);
					}
				}
			}
		public:
			void handleAndSubmitEvents() const {
				std::map<Axis, double> values;
				this->handleKeyState(values);
				this->handleJoysticks(values);
				this->handleGameControllers(values);
				this->submitValues(values);
			}
			
			void setKeyMapping(Uint8 keyCode, const Axis &axis, const double &value)
			{
				keyMap[keyCode] = std::make_tuple(axis, value);
			}
			
			void setJoystickMapping(std::shared_ptr<ReSDL::Joystick> joystick,
															int joystickAxis,
															const Axis &axis,
															AxisValueMappingS16 mapping)
			{
				joysticks[joystick][joystickAxis] = {mapping, axis};
			}
			
			void setGameControllerMapping(std::shared_ptr<ReSDL::GameController> controller,
																		SDL_GameControllerAxis controllerAxis,
																		const Axis &axis,
																		AxisValueMappingS16 mapping)
			{
				controllerMapping[controller][controllerAxis] = {mapping, axis};
			}
			
			void setAxisHandler(const Axis &axis, AxisInputHandlerFunc handler)
			{
				if(handler) {
					handlers[axis] = handler;
				}
				else {
					handlers.erase(axis);
				}
			}
			
			void setAxisOffValue(const Axis &axis, const double &value = std::numeric_limits<double>::quiet_NaN())
			{
				if(!isnan(value)) {
					offValues[axis] = value;
				}
				else {
					offValues.erase(axis);
				}
			}
			

		};

		
	}
}