cmake_minimum_required(VERSION 3.10)
project(AwesomeEngine VERSION 1.0)

# Default to an optimized build (big win for chunk meshing/generation).
# Debug with: cmake -DCMAKE_BUILD_TYPE=Debug ..
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)            # for clangd
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)
find_package(assimp QUIET)
if(assimp_FOUND)
  message(STATUS "Found Assimp: ${assimp_VERSION}")
else()
  message(WARNING "Assimp not found via find_package. Install it: brew install assimp")
endif()

# The tests option must be known before engine/ is processed, because engine/
# builds the `engine_test` (UNIT_TEST) library only when testing is on.
option(BUILD_TESTING "Build the unit tests" ON)
if(BUILD_TESTING)
  enable_testing()
endif()

add_subdirectory(engine)   # reusable static library: `engine` (+ `engine_test`)
add_subdirectory(game)     # the game executable, links `engine`

if(BUILD_TESTING)
  add_subdirectory(tests)  # `run-tests`, links `engine_test`
endif()
