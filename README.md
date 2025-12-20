# Awesome Engine

A modern OpenGL 4.1 graphics engine built with C++17, GLFW, and GLAD. This project demonstrates core graphics programming concepts including shader management, input handling, camera systems, texture loading, and advanced lighting techniques.

## Features

- **OpenGL 4.1 Core Profile** rendering
- **Advanced Lighting System**: 
  - Directional light (sun-like, constant direction)
  - Point light (light bulb, distance-based attenuation)
  - Spot light (flashlight, cone-shaped with smooth falloff)
- **Material System**: Phong lighting model with diffuse and specular textures
- **Input System**: Keyboard and mouse input handling with polling support
- **Camera System**: First-person camera with mouse look and smooth movement
- **Shader Management**: GLSL shader loading, compilation, and uniform caching
- **Texture Loading**: PNG/JPG texture support via stb_image with multiple texture support
- **Mesh System**: Abstract mesh base class with concrete implementations (Cube)
- **Model Loading**: Assimp integration for loading 3D model formats
- **Unit Testing**: Comprehensive test suite using doctest

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

### Step 1: Clone the Repository

```bash
git clone <repository-url>
cd awesome-engine
```

### Step 2: Create Build Directory

```bash
mkdir build
cd build
```

### Step 3: Configure with CMake

```bash
cmake ..
```

This will:

- Detect your compiler and system libraries
- Find GLFW, Assimp, and OpenGL
- Generate build files (Makefiles on Unix/macOS)

### Step 4: Build the Project

```bash
make
```

The compiled executable will be placed in the `bin/` directory as `awesome-engine`.

**Note**: Assets (shaders and textures) are automatically copied to `bin/` during the build process.

## Running the Application

After building, run the application from the project root:

```bash
./bin/awesome-engine
```

### Controls

- **W/A/S/D**: Move camera forward/left/backward/right
- **Mouse Movement**: Rotate camera view (first-person controls)
- **ESC**: Exit application

### Lighting Demo

The application demonstrates three types of lighting:

1. **Directional Light**: Simulates sunlight, affects all objects uniformly
2. **Point Light**: Positioned at the light cube, creates distance-based lighting
3. **Spot Light**: Follows the camera, creates a flashlight effect with smooth edges

## Running Tests

This project includes a comprehensive unit test suite for the `Input` and `Camera` classes.

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

## Project Structure

```
awesome-engine/
├── assets/              # Game assets (shaders, textures)
│   ├── shaders/        # GLSL shader files
│   │   ├── simple.vert        # Vertex shader
│   │   ├── simple.frag        # Basic fragment shader
│   │   ├── material.frag      # Phong lighting fragment shader
│   │   └── lighting.frag      # Light source shader
│   └── textures/       # Image files
├── bin/                 # Build output (executables and copied assets)
├── build/               # CMake build files (generated)
├── src/                 # Source code
│   ├── cameras/        # Camera system
│   │   └── Camera.h/cpp
│   ├── core/           # Core systems
│   │   ├── Config.h           # Configuration constants
│   │   ├── Input.h/cpp        # Input handling
│   │   └── TextureLoader.h/cpp # Texture management
│   ├── meshes/         # Mesh system
│   │   ├── Mesh.h             # Abstract mesh base class
│   │   └── Cube.h/cpp          # Cube mesh implementation
│   ├── shaders/        # Shader management
│   │   └── Shader.h/cpp
│   └── main.cpp        # Application entry point
├── tests/               # Unit tests
│   ├── CameraTests.cpp
│   └── InputTests.cpp
├── vendor/              # Third-party libraries
│   ├── doctest/        # Testing framework
│   ├── glad/           # OpenGL loader
│   ├── glm/            # Math library
│   └── loader/         # stb_image
├── CMakeLists.txt       # Build configuration
└── README.md           # This file
```

## Code Architecture

### Lighting System

The engine implements a comprehensive Phong lighting model with support for:

- **Material Properties**: Ambient, diffuse, and specular components with shininess
- **Multiple Light Sources**: Directional, point, and spot lights can be combined
- **Texture-Based Materials**: Diffuse and specular maps for realistic rendering
- **Smooth Falloff**: Spot lights feature smooth edge transitions using inner/outer cutoff angles

### Shader System

- **Uniform Caching**: Uniform locations are cached for performance
- **Error Handling**: Comprehensive error checking for shader compilation and linking
- **Type Safety**: Uses OpenGL types (GLuint, GLenum) for consistency

### Memory Management

- **RAII**: Resource management follows RAII principles
- **Rule of 5**: Classes properly handle copy/move semantics
- **Smart Pointers**: Uses `std::unique_ptr` for automatic memory management

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

### IDE Integration

This project generates a `compile_commands.json` file in the `build` directory, which enables:

- **IntelliSense** support in VS Code (with C++ extension)
- **CMake Tools** integration for VS Code
- **Clangd** language server support

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
