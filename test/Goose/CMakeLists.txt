cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 14)

add_executable(Goose
  src/main.cpp
  resource/gans.png
  resource/goose2.png
  resource/music.mid
  resource/puup.png)

set(RESOURCE_FILES
  resource/gans.png
  resource/goose2.png
  resource/music.mid
  resource/puup.png)

set_target_properties(Goose PROPERTIES
  RESOURCE "${RESOURCE_FILES}")

find_package(GLUT)
find_package(OpenGL)
target_link_libraries(Goose ${OPENGL_LIBRARIES})
target_link_libraries(Goose ${Engine_LIBRARY})