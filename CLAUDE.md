# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Build, clean rebuild, and run (from project root using Makefile)
make build          # Build only
make run            # Clean rebuild and run
make rebuild        # Rebuild without cleaning and run
make clean          # Remove bin/ directory

# Manual build (from project root)
cd build && cmake .. && make

# Rebuild after code changes
cd build && make

# Clean rebuild
cd build && rm -rf * && cmake .. && make

# Run the application (must cd into bin/ for asset paths and library linking)
cd bin && ./awesome-engine

# Run tests
./bin/run-tests

# Run tests with CTest (verbose)
cd build && ctest -V
```

## LSP Setup

CMake is configured with `CMAKE_EXPORT_COMPILE_COMMANDS ON` to generate `compile_commands.json` in `build/`. A symlink at the project root points to it for clangd. If LSP breaks after an Xcode/SDK update, do a clean rebuild: `cd build && rm -rf * && cmake .. && make`.

## Dependencies

Install system dependencies via Homebrew:
```bash
brew install glfw assimp
```

Vendor libraries (GLAD, GLM, stb_image, doctest) are included in `vendor/`.

## Architecture

### Source Organization (`src/`)
- **core/**: Config constants, Input handling (keyboard/mouse polling), TextureLoader
- **cameras/**: First-person Camera with mouse look
- **meshes/**: Abstract Mesh base class, Cube, Model (Assimp-based 3D loading)
- **rendering/**: Shader management, PostProcessing with Strategy pattern, Skybox
- **main.cpp**: Application entry point and render loop

### Key Patterns

**Shader Class**: Uses Rule of 5 - copy constructor/assignment are deleted (OpenGL handles can't be copied). Pass Shader by reference, not value.

**PostProcessing**: Uses Strategy pattern - effect strategies inherit from `PostProcessEffectStrategy` and implement rendering effects (blur, invert, edge detection).

**Include Order**: GLAD must be included before GLFW to avoid OpenGL header conflicts. Use a blank line to prevent formatters from reordering:
```cpp
#include <glad/glad.h>

#include <GLFW/glfw3.h>
```

### Assets
- Shaders: `assets/shaders/` (GLSL `.vert`, `.frag`, `.geo` files)
- Textures: `assets/textures/`
- Models: `assets/models/`

Assets are automatically copied to `bin/` during build.

## Testing

Tests use doctest framework. Test files are in `tests/`. The `UNIT_TEST` macro is defined for test builds to expose private members.

```bash
# Run specific test output options
./bin/run-tests -s          # Show all assertions
./bin/run-tests -s -d       # Show with execution time
./bin/run-tests --list-test-cases  # List all tests
```
