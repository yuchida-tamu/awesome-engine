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
    src/shaders/Shader.cpp
    vendor/loader/src/stb_image.cpp
    src/cameras/Camera.cpp
)

# Tell our target where to find its public header files
# This is especially important for IntelliSense to work correctly
target_include_directories(awesome-engine PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/glad/include
    ${CMAKE_SOURCE_DIR}/vendor/loader/include
    ${CMAKE_SOURCE_DIR}/vendor/glm
)

# Link the libraries.
# The order is important here. We link our executable to GLFW.
# GLFW then needs to link to the system's OpenGL library.
target_link_libraries(awesome-engine PRIVATE glfw OpenGL::GL)


# Define the output directory for the shaders
set(SHADER_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)

# Define the output directory for the textures
set(TEXTURE_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/textures)

# Create the output directory
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})
file(MAKE_DIRECTORY ${TEXTURE_OUTPUT_DIR})

# Find all shader files in the NEW asset source directory
file(GLOB SHADER_FILES "${CMAKE_SOURCE_DIR}/assets/shaders/*.vert" "${CMAKE_SOURCE_DIR}/assets/shaders/*.frag")

# Find all texture files in the asset directory
file(GLOB TEXTURE_FILES "${CMAKE_SOURCE_DIR}/assets/textures/*.png" "${CMAKE_SOURCE_DIR}/assets/textures/*.jpg")


# For each shader file, add a command to copy it to the output directory
foreach(SHADER_FILE ${SHADER_FILES})
    add_custom_command(
        TARGET awesome-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${SHADER_FILE}
        ${SHADER_OUTPUT_DIR}
    )
endforeach()

# For each texture file, add a command to copy it to the output directory
foreach(TEXTURE_FILE ${TEXTURE_FILES})
    add_custom_command(
        TARGET awesome-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${TEXTURE_FILE}
        ${TEXTURE_OUTPUT_DIR}
    )
endforeach()

# ===================================================================
# Unit Testing Setup
# ===================================================================

# We only want to build tests if this option is enabled.
option(BUILD_TESTING "Build the unit tests" ON)

if(BUILD_TESTING)
    # Enable the CTest module, which helps run tests
    enable_testing()

    # Add a new executable for our tests.
    # It needs to compile the test files AND the source files they depend on.
    add_executable(run-tests
        tests/InputTests.cpp
        tests/CameraTests.cpp
        src/core/Input.cpp
        src/cameras/Camera.cpp
    )

    # Add a compile definition to expose private members for testing
    target_compile_definitions(run-tests PRIVATE UNIT_TEST)

    # The test executable needs to know where to find headers, just like our main app.
    target_include_directories(run-tests PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/vendor/doctest
        # We need GLFW headers because Input.h includes it
        ${CMAKE_SOURCE_DIR}/vendor/glfw/include #<-- You might need to adjust this path
    )

    # Link the test executable to the libraries it needs.
    # We need GLFW because the Input class calls GLFW functions.
    target_link_libraries(run-tests PRIVATE glfw)

    # Add the test to CTest so we can run it easily
    add_test(NAME InputTests COMMAND run-tests)
endif()