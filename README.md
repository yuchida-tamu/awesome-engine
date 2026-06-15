# Awesome Engine

A modern OpenGL 4.1 graphics engine built with C++17, GLFW, and GLAD, evolving toward voxel rendering. Started as a learning project to explore core graphics programming concepts — shaders, cameras, input, texturing, and lighting — and is now growing a voxel pipeline on top of that foundation.

> **Work in progress.** Internal structure and APIs change frequently. See the [Roadmap](#roadmap) for what's done and what's next.

## Features

- **OpenGL 4.1 Core Profile** rendering
- **Phong lighting** with directional, point, and spot lights
- **First-person camera** with mouse look and keyboard movement
- **Texturing** via stb_image (PNG/JPG)
- **Model loading** through Assimp
- **Voxel meshing** with greedy meshing for chunk geometry
- **Unit testing** with doctest

## Roadmap

This is an active work-in-progress. Internal structure and APIs change
frequently. The current focus is voxel rendering.

- [x] Core renderer: shaders, camera, input, textures, lighting
- [x] Mesh abstraction + model loading (Assimp)
- [x] Chunk + greedy meshing (voxel geometry)
- [ ] Render chunks within the Scene
- [ ] Heightmap-based terrain generation
- [ ] Multi-chunk world streaming
- [ ] Block types & texturing
- [ ] Lighting / ambient occlusion for voxels

_This section is updated as the project progresses._

## Prerequisites

### Required Software

- **CMake** (version 3.10 or higher)
- **C++17 compatible compiler** (Clang, GCC, or MSVC)
- **OpenGL 4.1** compatible graphics driver
- **Homebrew** (macOS) or appropriate package manager for your platform

### macOS Specific Requirements

- macOS 10.9 or later (for OpenGL 4.1 support)
- Xcode Command Line Tools

## Dependencies

This project uses the following external libraries, which are managed via Homebrew (macOS) or included as vendor libraries:

### System Dependencies (via Homebrew)

- **GLFW** - Window and input management
- **Assimp** - 3D model loading library
- **OpenGL** - Graphics API (provided by the system)

Install with:

```bash
brew install glfw assimp
```

### Vendor Libraries (Included)

The following libraries are included in the `vendor/` directory:

- **GLAD** - OpenGL function loader
- **GLM** - OpenGL Mathematics library
- **stb_image** - Image loading library
- **doctest** - Unit testing framework

## Building the Project

### Quick Start (using Makefile)

```bash
git clone <repository-url>
cd awesome-engine
make run        # Clean rebuild and run
```

### Makefile Targets

| Target       | Description                        |
|--------------|------------------------------------|
| `make build` | Build only (no run)                |
| `make run`   | Clean rebuild and run              |
| `make rebuild` | Rebuild without cleaning and run |
| `make clean` | Remove `bin/` directory            |

### Manual Build

```bash
mkdir -p build && cd build
cmake ..
make
```

The compiled executable will be placed in the `bin/` directory as `awesome-engine`.

**Note**: Assets (shaders, textures, and models) are automatically copied to `bin/` during the build process.

## Running the Application

After building, run the application from the `bin/` directory:

```bash
cd bin && ./awesome-engine
```

### Controls

- **W/A/S/D**: Move camera forward/left/backward/right
- **Mouse Movement**: Rotate camera view (first-person controls)
- **ESC**: Exit application

## Running Tests

This project includes a unit test suite built with doctest.

### Building Tests

Tests are built automatically when you run `make` in the `build` directory. The test executable is created as `bin/run-tests`.

### Running Tests

#### Option 1: Using CTest (Recommended)

From the `build` directory:

```bash
ctest
```

For verbose output:

```bash
ctest -V
```

#### Option 2: Running Directly

From the project root:

```bash
./bin/run-tests
```

### Test Output Options

The test executable supports several output modes:

```bash
# Show all successful assertions and test case names
./bin/run-tests -s

# Show test case names, assertions, and execution time
./bin/run-tests -s -d

# List all test cases without running them
./bin/run-tests --list-test-cases

# Show help
./bin/run-tests --help
```

## Development

### Rebuilding After Changes

If you modify source files, simply run `make` again in the `build` directory:

```bash
cd build
make
```

If you modify `CMakeLists.txt` or add new source files, you need to reconfigure:

```bash
cd build
cmake ..
make
```

### Clean Build

To perform a clean rebuild:

```bash
cd build
rm -rf *
cmake ..
make
```

### IDE / LSP Integration

CMake is configured with `CMAKE_EXPORT_COMPILE_COMMANDS ON`, which generates a `compile_commands.json` in the `build/` directory. A symlink at the project root points to it so clangd can find it automatically.

This enables:

- **Clangd** language server support (autocomplete, go-to-definition, diagnostics)
- **IntelliSense** support in VS Code (with C++ extension)
- **CMake Tools** integration for VS Code

If LSP stops working after an Xcode or macOS update, do a clean rebuild to regenerate `compile_commands.json` with the new SDK paths:

```bash
cd build && rm -rf * && cmake .. && make
```

## Troubleshooting

### GLFW Not Found

If CMake cannot find GLFW:

```bash
# macOS: Ensure GLFW is installed via Homebrew
brew install glfw

# Verify installation
brew list glfw
```

### Assimp Not Found

If CMake cannot find Assimp:

```bash
# macOS: Ensure Assimp is installed via Homebrew
brew install assimp

# Verify installation
brew list assimp
```

### OpenGL Version Errors

If you encounter "Failed to create GLFW window":

- Ensure your graphics drivers support OpenGL 4.1
- On macOS, OpenGL 4.1 is the maximum supported version
- Verify your system meets the minimum requirements

### OpenGL/SDK Path Errors After Xcode Update

If you see errors like `Imported target "OpenGL::GL" includes non-existent path` referencing an old SDK (e.g., `MacOSX15.5.sdk`), the CMake cache is stale. Do a clean rebuild:

```bash
cd build && rm -rf * && cmake .. && make
```

### Test Compilation Errors

If tests fail to compile:

- Ensure `UNIT_TEST` macro is being defined (handled automatically by CMake)
- Check that all test dependencies are included in `CMakeLists.txt`
- Verify doctest header is in `vendor/doctest/doctest.h`

### Runtime Library Errors (macOS)

If you get "library not loaded" errors:

- Ensure Assimp is installed via Homebrew: `brew install assimp`
- The CMake configuration sets RPATH automatically for macOS
- Verify the library path: `otool -L bin/awesome-engine`

## License

[Add your license here]

## Contributing

[Add contribution guidelines here]
