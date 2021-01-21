cmake_minimum_required(VERSION 3.2)

project(ReSDL)

set (CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
#find_package(SDL2_image REQUIRED)
#find_package(SDL2_mixer REQUIRED)

add_library(ReSDL STATIC 
  includes/ReSDL/ReSDL.h
  includes/ReSDL/ReSDLCommon.h
  includes/ReSDL/ReSDLCore.h
  includes/ReSDL/ReSDLTypes.h
#  includes/ReSDL/ReSDLImage.h
  src/ReSDLCore.cpp
  src/ReSDLTypes.cpp
 "includes/ReSDL/Surface.h")

target_link_libraries(ReSDL SDL2::SDL2-static)
target_include_directories(ReSDL
  PUBLIC includes
  PUBLIC ${SDL2_INCLUDE_DIR})
  
set(CMAKE_FOLDER tests)
add_subdirectory(test/Engine)
add_subdirectory(test/Game1)