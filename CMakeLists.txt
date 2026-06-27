cmake_minimum_required(VERSION 3.10)

# Define the project
project(AwesomeEngine VERSION 1.0)

# Default to an optimized build (huge win for chunk meshing/generation). Build
# unoptimized for debugging with: cmake -DCMAKE_BUILD_TYPE=Debug ..
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

# Export compile commands for LSP (clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# Find the OpenGL package provided by CMake
find_package(OpenGL REQUIRED)

# Find GLFW using its own config files (provided by Homebrew)
find_package(glfw3 REQUIRED)

# Find FreeType for text rendering
find_package(Freetype REQUIRED)

# Find Assimp using CMake config files (provided by Homebrew)
find_package(assimp QUIET)
if(assimp_FOUND)
    message(STATUS "Found Assimp: ${assimp_VERSION}")
else()
    message(WARNING "Assimp not found via find_package. You may need to install it: brew install assimp")
endif()

# ===================================================================
# Engine Library Setup
# ===================================================================
set(ENGINE_SOURCES 
    vendor/glad/src/glad.c
    vendor/loader/src/stb_image.cpp
    src/cameras/Camera.cpp
    src/core/Input.cpp
    src/core/Window.cpp
    src/core/TextureLoader.cpp
    src/meshes/Model.cpp
    src/meshes/Mesh.cpp
    src/meshes/Cube.cpp
    src/rendering/PostProcessing.cpp
    src/rendering/PostProcessBlurEffectStrategy.cpp
    src/rendering/PostProcessInvertEffectStrategy.cpp
    src/rendering/PostProcessEdgeEffectStrategy.cpp
    src/rendering/Skybox.cpp
    src/rendering/Shader.cpp
    src/scene/WorldSpaceGizmo.cpp
    src/scene/GridGizmo.cpp
    src/scene/Scene.cpp
    src/scene/GameObject.cpp
    src/scene/RenderComponent.cpp
    src/scene/CameraController.cpp
    src/ui/UIManager.cpp
    src/ui/TextElement.cpp
    src/ui/ButtonElement.cpp
    src/ui/DebugPanel.cpp
    src/voxel/VoxelChunk.cpp
    src/voxel/ChunkMesher.cpp
    src/voxel/GreedyMesher.cpp
    src/voxel/TerrainGenerator.cpp
    src/world/World.cpp
)
set(ENGINE_INCLUDES   ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/vendor/glad/include
  ${CMAKE_SOURCE_DIR}/vendor/loader/include
  ${CMAKE_SOURCE_DIR}/vendor/glm
  ${CMAKE_SOURCE_DIR}/vendor/FastNoiseLite
)
add_library(engine STATIC ${ENGINE_SOURCES}) # production, no UNIT_TEST
target_include_directories(engine PUBLIC ${ENGINE_INCLUDES})
target_link_libraries(engine PUBLIC glfw OpenGL::GL Freetype::Freetype)
if(assimp_FOUND)
    target_include_directories(engine PUBLIC ${assimp_INCLUDE_DIRS})
endif()
add_executable(awesome-engine src/main.cpp)
target_link_libraries(awesome-engine PRIVATE engine)

if(assimp_FOUND)
  target_link_libraries(awesome-engine PRIVATE assimp::assimp)
endif()

# Define the output directory for the shaders
set(SHADER_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)

# Define the output directory for the textures
set(TEXTURE_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/textures)

# Create the output directory
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})
file(MAKE_DIRECTORY ${TEXTURE_OUTPUT_DIR})

# Find all files in the shaders directory (copy everything, not just .vert/.frag)
file(GLOB SHADER_FILES "${CMAKE_SOURCE_DIR}/assets/shaders/*")

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

# Define the output directory for fonts
set(FONT_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fonts)
file(MAKE_DIRECTORY ${FONT_OUTPUT_DIR})

# Find all font files
file(GLOB FONT_FILES "${CMAKE_SOURCE_DIR}/assets/fonts/*.ttf" "${CMAKE_SOURCE_DIR}/assets/fonts/*.otf")

# Copy each font file to the output directory
foreach(FONT_FILE ${FONT_FILES})
    add_custom_command(
        TARGET awesome-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${FONT_FILE}
        ${FONT_OUTPUT_DIR}
    )
endforeach()

# Define the output directory for the models
set(MODEL_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/models)

# Create the output directory
file(MAKE_DIRECTORY ${MODEL_OUTPUT_DIR})

# Copy all model files recursively to bin directory (preserves subdirectories)
# This copies the entire models folder structure
add_custom_command(
    TARGET awesome-engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets/models
    ${MODEL_OUTPUT_DIR}
    COMMENT "Copying model files to bin directory"
)

# On macOS, copy Assimp library to bin directory and set RPATH
if(APPLE AND assimp_FOUND)
    # Get Homebrew prefix (usually /opt/homebrew or /usr/local)
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE HOMEBREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    # Get the actual library path (resolve symlink)
    execute_process(
        COMMAND python3 -c "import os; print(os.path.realpath('${HOMEBREW_PREFIX}/lib/libassimp.5.dylib'))"
        OUTPUT_VARIABLE ASSIMP_LIB_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # Fallback to the symlink path if Python failed
    if(NOT ASSIMP_LIB_PATH)
        set(ASSIMP_LIB_PATH ${HOMEBREW_PREFIX}/lib/libassimp.5.dylib)
    endif()
    
    # Copy Assimp library to bin directory (copy_if_different follows symlinks)
    add_custom_command(
        TARGET awesome-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ASSIMP_LIB_PATH}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libassimp.5.dylib
        COMMENT "Copying Assimp library to bin directory"
    )
    
    # Re-sign the copied library with adhoc signature (required for macOS)
    add_custom_command(
        TARGET awesome-engine POST_BUILD
        COMMAND codesign --force --sign - ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libassimp.5.dylib
        COMMENT "Re-signing Assimp library"
    )
    
    # Set RPATH to @executable_path so it finds the copied library
    set_target_properties(awesome-engine PROPERTIES
        INSTALL_RPATH "@executable_path"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    
    # Use install_name_tool to change the library path in the binary
    add_custom_command(
        TARGET awesome-engine POST_BUILD
        COMMAND install_name_tool -change "@rpath/libassimp.5.dylib" "@executable_path/libassimp.5.dylib" $<TARGET_FILE:awesome-engine>
        COMMENT "Updating library path in binary"
    )
endif()

# ===================================================================
# Unit Testing Setup
# ===================================================================

# We only want to build tests if this option is enabled.
option(BUILD_TESTING "Build the unit tests" ON)

if(BUILD_TESTING)
    # Enable the CTest module, which helps run tests
    enable_testing()

    add_library(engine_test STATIC ${ENGINE_SOURCES})
    target_include_directories(engine_test PUBLIC ${ENGINE_INCLUDES})
    target_link_libraries(engine_test PUBLIC glfw OpenGL::GL Freetype::Freetype)
    target_compile_definitions(engine_test PUBLIC UNIT_TEST)


    # Add a new executable for our tests.
    # It needs to compile the test files AND the source files they depend on.
    add_executable(run-tests
        tests/InputTests.cpp
        tests/CameraTests.cpp
        tests/EventBusTests.cpp
        tests/TransformComponentTests.cpp
        tests/CameraControllerTests.cpp
        tests/GameObjectTests.cpp
        tests/SceneTests.cpp
        tests/RenderContextTests.cpp
        tests/UIElementTests.cpp
        tests/UIManagerTests.cpp
        tests/TextElementTests.cpp
        tests/ButtonElementTests.cpp
        tests/ChunkTests.cpp
        tests/ChunkMesherTests.cpp
        tests/GreedyMesherTests.cpp
        tests/TerrainGeneratorTests.cpp
        tests/CoordsTests.cpp
        tests/FpsCounterTests.cpp
        tests/ReconcileTests.cpp
        tests/DesiredSetTests.cpp)

    target_include_directories(run-tests PRIVATE
      ${CMAKE_SOURCE_DIR}/vendor/doctest
      ${CMAKE_SOURCE_DIR}/tests
    )
    
    # Link the test executable to the libraries it needs.
    # We need GLFW because the Input class calls GLFW functions.
    # target_link_libraries(run-tests PRIVATE glfw Freetype::Freetype)
    target_link_libraries(run-tests PRIVATE engine_test)


    # Add the test to CTest so we can run it easily
    add_test(NAME AllTests COMMAND run-tests)
endif()
