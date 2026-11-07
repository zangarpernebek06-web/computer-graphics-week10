cmake_minimum_required(VERSION 3.15)
project(cg_week10 CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(cg main.cpp)
target_link_libraries(cg PRIVATE glfw GLEW::GLEW OpenGL::GL)
