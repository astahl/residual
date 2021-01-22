//
//  AxisInputManager.h
//  libsdlTest
//
//  Created by Andreas Stahl on 23.02.15.
//  Copyright (c) 2015 Andreas. All rights reserved.
//

#pragma once
#include <Engine/Utilities.h>

namespace Engine {
	namespace Input {
		
		enum class Axis {
			Main_X, Main_Y, Main_Z,
			Secondary_X, Secondary_Y, Secondary_Z,
			Tertiary_X, Tertiary_Y, Tertiary_Z,
			Aux_0,
			Aux_1,
			Aux_2,
			Aux_3,
			Aux_4,
			Aux_5,
			Aux_6,
			Aux_7,
			Aux_8,
			Aux_9
		};

		enum class Radial {
			Main, Secondary, Tertiary, 
			Aux_0,
			Aux_1,
			Aux_2,
			Aux_3,
			Aux_4,
			Aux_5,
			Aux_6,
			Aux_7,
			Aux_8,
			Aux_9
		};
		
		using AxisInputHandlerFunc = std::function<void(const Axis, const double)>;
		using AxisValueMappingS16 = std::function<double(Sint16)>;
		
		struct KeyMapping {
			Axis axis;
			double maxValue;
			double offValue;
			double minValue;
		};

		struct S16Mapping {
			AxisValueMappingS16 mappingFunc;
			Axis axis;
		};

		using RadialInputHandlerFunc = std::function<void(Radial, Vec2d)>;
		using RadialValueMappingS16 = std::function<Vec2d(Sint16, Sint16)>;

		struct S16RadialMapping {
			RadialValueMappingS16 mappingFunc;
			Radial radial;
		};

		struct KeyCross {
			Uint8 up, down, left, right;
		};

		struct RadialKeyMapping {
			Radial radial;
		};

		class AxisInputManager
		{
			using SDL_JoystickAxisId = int;
			std::map<Uint8, KeyMapping> keys;
			std::map<std::pair<Uint8, Uint8>, KeyMapping> keyPairs;
			std::map<Radial, KeyCross> keyQuads;
			std::map<std::shared_ptr<ReSDL::Joystick>, std::map<SDL_JoystickAxisId, S16Mapping>> joysticks;
			std::map<std::shared_ptr<ReSDL::GameController>, std::map<SDL_GameControllerAxis, S16Mapping>> controllerMapping;
			std::map<std::shared_ptr<ReSDL::GameController>, std::map<std::pair<SDL_GameControllerAxis, SDL_GameControllerAxis>, S16RadialMapping>> controllerRadialMapping;
			std::map<Axis, AxisInputHandlerFunc> handlers;
			std::map<Radial, RadialInputHandlerFunc> radialHandlers;
			
			const Sint16 deadZone = 1000;

			void handleKeyState(std::map<Axis, double> &values, std::map<Radial, Vec2d>& radialValues) const
			{
				const Uint8 *state = SDL_GetKeyboardState(NULL);
				

				for (const auto& [key, mapping] : keys)
				{
					values[mapping.axis] = state[key] ? mapping.maxValue : mapping.offValue;
				}

				for (const auto& [ keypair, mapping ] : keyPairs)
				{
					values[mapping.axis] = state[keypair.first] ? mapping.minValue : state[keypair.second] ? mapping.maxValue : mapping.offValue;
				}
				
				for (const auto& [ radial, keycross] : keyQuads)
				{
					Vec2d value{};
					const bool
						l = state[keycross.left],
						r = state[keycross.right],
						u = state[keycross.up],
						d = state[keycross.down];

					const double val = (l || r) && (u || d) ? 0.707 : 1.0;
					value.x() = l ? -val : r ?  val : 0.0;
					value.y() = d ?  val : u ? -val : 0.0;
					
					radialValues[radial] = value;
				}
			}
			
			void handleJoysticks(std::map<Axis, double> &values) const
			{
				for(const auto& [ joystick, mappings ] : joysticks)
				{
					for(const auto& [ joyAxis, mapping ] : mappings)
					{
						const Sint16 axisValue = joystick->getAxis(joyAxis);
						if (abs(axisValue) > deadZone) {
							values[mapping.axis] = mapping.mappingFunc(axisValue);
						}
					}
				}
			}
			
			void handleGameControllers(std::map<Axis, double> &values, std::map<Radial, Vec2d>& radialValues) const
			{
				for(const auto& [ controller, mappings ] : controllerMapping)
				{
					for(const auto& [ conAxis, mapping ] : mappings)
					{
						const Sint16 axisValue = controller->getAxis(conAxis);
						if (abs(axisValue) > deadZone) {
							values[mapping.axis] = mapping.mappingFunc(axisValue);
						}
					}
				}

				for (const auto& [controller, mappings] : controllerRadialMapping)
				{
					for (const auto& [conAxis, mapping] : mappings)
					{
						const auto valueX = controller->getAxis(conAxis.first);
						const auto valueY = controller->getAxis(conAxis.second);
						if (abs(valueX) > deadZone || abs(valueY) > deadZone) {
							radialValues[mapping.radial] = mapping.mappingFunc(valueX, valueY);
						}
					}
				}
			}
			
			void submitValues(const std::map<Axis, double> &values) const
			{
				for(const auto& [axis, handler] : handlers) {
					if(values.count(axis))
					{
						handler(axis, values.at(axis));
					}
				}
			}

			void submitRadialValues(const std::map<Radial, Vec2d>& values) const
			{
				for (const auto& [radial, handler] : radialHandlers) {
					if (values.count(radial))
					{
						handler(radial, values.at(radial));
					}
				}
			}

		public:
			void handleAndSubmitEvents() const {
				std::map<Axis, double> values{};
				std::map<Radial, Vec2d> radialValues{};
				this->handleKeyState(values, radialValues);
				this->handleJoysticks(values);
				this->handleGameControllers(values, radialValues);
				this->submitValues(values);
				submitRadialValues(radialValues);
			}
			


			void setKeyMapping(const Axis& axis, Uint8 keyCode, double offValue, double maxValue)
			{
				keys[keyCode] = KeyMapping{ axis, maxValue, offValue, std::numeric_limits<double>::quiet_NaN() };
			}
			
			void setKeyMapping(const Axis& axis, Uint8 keyCodeMin, Uint8 keyCodeMax, double minValue, double offValue, double maxValue)
			{
				keyPairs[std::make_pair(keyCodeMin, keyCodeMax)] = KeyMapping{ axis, maxValue, offValue, minValue };
			}

			void setKeyMapping(const Radial& radial, Uint8 keyCodeUp, Uint8 keyCodeDown, Uint8 keyCodeLeft, Uint8 keyCodeRight)
			{
				keyQuads[radial] = { keyCodeUp, keyCodeDown, keyCodeLeft, keyCodeRight }; 
			}

			void setJoystickMapping(std::shared_ptr<ReSDL::Joystick> joystick,
				SDL_JoystickAxisId joystickAxis,
				Axis axis,
				AxisValueMappingS16 mapping)
			{
				joysticks[joystick][joystickAxis] = S16Mapping{ mapping, axis };
			}
			
			void setGameControllerMapping(std::shared_ptr<ReSDL::GameController> controller,
				SDL_GameControllerAxis controllerAxis,
				Axis axis,
				AxisValueMappingS16 mapping)
			{
				controllerMapping[controller][controllerAxis] = S16Mapping{ mapping, axis };
			}

			void setGameControllerMapping(std::shared_ptr<ReSDL::GameController> controller,
				SDL_GameControllerAxis controllerAxisX,
				SDL_GameControllerAxis controllerAxisY,
				Radial radial,
				RadialValueMappingS16 mapping)
			{
				controllerRadialMapping[controller][std::make_pair(controllerAxisX, controllerAxisY)] = S16RadialMapping{ mapping, radial };
			}
			
			void setAxisHandler(Axis axis, AxisInputHandlerFunc handler)
			{
				if(handler) {
					handlers[axis] = handler;
				}
				else {
					handlers.erase(axis);
				}
			}

			void setRadialHandler(Radial radial, RadialInputHandlerFunc handler)
			{
				if (handler) {
					radialHandlers[radial] = handler;
				}
				else {
					radialHandlers.erase(radial);
				}
			}

		};

		
	}
}