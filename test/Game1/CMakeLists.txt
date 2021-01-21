cmake_minimum_required(VERSION 3.2)

set (CMAKE_CXX_STANDARD 17)

add_executable(Game
	src/Main.cpp)

find_package(GLUT)
find_package(OpenGL)
target_link_libraries(Game ${OPENGL_LIBRARIES})
target_link_libraries(Game ${Engine_LIBRARY})