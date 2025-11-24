cmake_minimum_required(VERSION 3.10)

# Define the project
project(AwesomeEngine VERSION 1.0)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# Find the OpenGL package provided by CMake
find_package(OpenGL REQUIRED)

# Find GLFW using its own config files (provided by Homebrew)
find_package(glfw3 REQUIRED)

# Add our executable target with all its source files
add_executable(awesome-engine
    src/main.cpp
    src/core/Input.cpp
    vendor/glad/src/glad.c
)

# Tell our target where to find its public header files
# This is especially important for IntelliSense to work correctly
target_include_directories(awesome-engine PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/glad/include
)

# Link the libraries.
# The order is important here. We link our executable to GLFW.
# GLFW then needs to link to the system's OpenGL library.
target_link_libraries(awesome-engine PRIVATE glfw OpenGL::GL)