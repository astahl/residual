cmake_minimum_required(VERSION 3.2)

set (CMAKE_CXX_STANDARD 14)

add_library(Engine STATIC
  	includes/Engine/Engine.h
  	includes/Engine/Utilities.h
	includes/Engine/Input/AxisInputManager.h
	includes/Engine/Input/ButtonInputManager.h
	includes/Engine/Input/EventManager.h
	src/Engine.cpp
)

target_link_libraries(Engine PUBLIC ReSDL)
target_include_directories(Engine PUBLIC includes)

set(Engine_LIBRARY Engine PARENT_SCOPE)