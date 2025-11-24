cmake_minimum_required(VERSION 3.10)

# Define the project
project(AwesomeEngine VERSION 1.0)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the OpenGL package provided by CMake
find_package(OpenGL REQUIRED)

# Find GLFW using its own config files (provided by Homebrew)
find_package(glfw3 REQUIRED)

# Add our executable target
add_executable(awesome-engine main.cpp)

# Link the libraries.
# The order is important here. We link our executable to GLFW.
# GLFW then needs to link to the system's OpenGL library.
target_link_libraries(awesome-engine PRIVATE glfw OpenGL::GL)