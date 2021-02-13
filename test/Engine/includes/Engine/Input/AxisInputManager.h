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
		
		enum class Button {
			A, B,
			X, Y,
			Select,
			Start,
			DPad_Up, DPad_Down, DPad_Left, DPad_Right,
			Home,
			Shoulder_Left, Shoulder_Right,
			Stick_Left, Stick_Right
		};


		enum class ButtonState {
			Off = 0,
			Push 		= 0b001, 
			Hold 		= 0b010,
			Release 	= 0b100,
			All = Push | Hold | Release
		};

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

		template<typename T>
		struct Cross {
			T up, down, left, right;
		};

		using KeyCross = Cross<Uint8>;
		using GameControllerButtonCross = Cross<SDL_GameControllerButton>;

		struct RadialControllerMapping {
			Radial radial;
			GameControllerButtonCross cross;
		};

		using ButtonStateHandlerFunc = std::function<void(Button, ButtonState)>;
		struct ButtonStateHandler {
			ButtonState filter;
			ButtonState previousState;
			ButtonStateHandlerFunc handler;

			void handle(Button button, ButtonState state) {
				if (previousState == state) {
					handler(button, state);
				} else if (previousState == ButtonState::Off) {
					// transition state
					handler(button, ButtonState::Push);
					//handler(button, ButtonState::Hold);
				} else if (previousState == ButtonState::Hold) {
					handler(button, ButtonState::Release);
					//handler(button, ButtonState::Off);
				}

				previousState = state;
			}
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
			std::map<std::shared_ptr<ReSDL::GameController>, RadialControllerMapping> controllerRadialButtonMapping;
			std::map<Axis, AxisInputHandlerFunc> handlers;
			std::map<Radial, RadialInputHandlerFunc> radialHandlers;

			std::map<Uint8, Button> keyButtons;
			std::map<std::shared_ptr<ReSDL::Joystick>, std::map<Uint8, Button>> joystickButtons;
			std::map<std::shared_ptr<ReSDL::GameController>, std::map<SDL_GameControllerButton, Button>> controllerButtons;
			
			std::map<Button, ButtonStateHandler> buttonHandlers;
			
			const Sint16 deadZone = 1000;

			void handleKeyState(std::map<Axis, double> &values, std::map<Radial, Vec2d>& radialValues, std::map<Button, ButtonState>& buttonValues) const
			{
				const Uint8 *state = SDL_GetKeyboardState(NULL);
				
				for (const auto& [key, button] : keyButtons) 
				{
					if (state[key]) {
						buttonValues[button] = ButtonState::Hold;
					}
				}

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
			
			void handleGameControllers(std::map<Axis, double> &values, std::map<Radial, Vec2d>& radialValues, std::map<Button, ButtonState>& buttonValues) const
			{
				for(const auto& [ controller, mappings ] : controllerButtons)
				{
					for(const auto& [ controllerButton, button ] : mappings)
					{					
						if (controller->getButton(controllerButton)) {
							buttonValues[button] = ButtonState::Hold;
						}
					}
				}

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

				for (const auto& [ controller, mapping] : controllerRadialButtonMapping)
				{
					Vec2d value{};
					const bool
						l = controller->getButton(mapping.cross.left),
						r = controller->getButton(mapping.cross.right),
						u = controller->getButton(mapping.cross.up),
						d = controller->getButton(mapping.cross.down);

					const double val = (l || r) && (u || d) ? 0.707 : 1.0;
					value.x() = l ? -val : r ?  val : 0.0;
					value.y() = d ?  val : u ? -val : 0.0;
					
					radialValues[mapping.radial] = value;
				}
			}
			
			void submitValues(const std::map<Axis, double> &values, const std::map<Radial, Vec2d>& radialValues, const std::map<Button, ButtonState>& buttonValues)
			{
				for(const auto& [axis, handler] : handlers) {
					if(values.count(axis))
					{
						handler(axis, values.at(axis));
					}
				}

				for (const auto& [radial, handler] : radialHandlers) {
					if (radialValues.count(radial))
					{
						handler(radial, radialValues.at(radial));
					}
				}

				for (auto& [button, handler] : buttonHandlers) {
					if (buttonValues.count(button))
					{
						handler.handle(button, buttonValues.at(button));
					}
				}
			}

		public:
			void handleAndSubmitEvents() {
				std::map<Axis, double> values{};
				std::map<Radial, Vec2d> radialValues{};
				std::map<Button, ButtonState> buttonValues{};
				for (auto& [button, handler] : buttonHandlers) {
					buttonValues[button] = ButtonState::Off;
				}
				this->handleKeyState(values, radialValues, buttonValues);
				this->handleJoysticks(values);
				this->handleGameControllers(values, radialValues, buttonValues);
				this->submitValues(values, radialValues, buttonValues);
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

			void setGameControllerMapping(std::shared_ptr<ReSDL::GameController> controller,
				SDL_GameControllerButton buttonUp,
				SDL_GameControllerButton buttonDown,
				SDL_GameControllerButton buttonLeft,
				SDL_GameControllerButton buttonRight,
				Radial radial)
			{
				controllerRadialButtonMapping[controller] = { radial, GameControllerButtonCross{buttonUp, buttonDown, buttonLeft, buttonRight }};
			}

			void setGameControllerMapping(std::shared_ptr<ReSDL::GameController> controller,
				SDL_GameControllerButton controllerButton, 
				Button button)
			{
				controllerButtons[controller][controllerButton] = button;
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

			void setButtonHandler(Button button, ButtonStateHandlerFunc handler, ButtonState filter = ButtonState::All) 
			{
				if (handler) {
					buttonHandlers[button] = { filter, ButtonState::Off, handler };
				}
				else {
					buttonHandlers.erase(button);
				}
			}

		};

		
	}
}