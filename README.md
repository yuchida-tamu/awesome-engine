# Awesome Engine

A modern OpenGL 4.1 graphics engine built with C++17, GLFW, and GLAD. This project demonstrates core graphics programming concepts including shader management, input handling, camera systems, and texture loading.

## Features

- **OpenGL 4.1 Core Profile** rendering
- **Input System**: Keyboard and mouse input handling with polling support
- **Camera System**: First-person camera with mouse look
- **Shader Management**: GLSL shader loading and compilation
- **Texture Loading**: PNG/JPG texture support via stb_image
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
- **OpenGL** - Graphics API (provided by the system)

Install with:

```bash
brew install glfw
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
- Find GLFW and OpenGL
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
- **Mouse Movement**: Rotate camera view
- **ESC**: Exit application

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
│   └── textures/       # Image files
├── bin/                 # Build output (executables and copied assets)
├── build/               # CMake build files (generated)
├── src/                 # Source code
│   ├── camera/         # Camera system
│   ├── core/           # Core systems (Input)
│   ├── shader/         # Shader management
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

## License

[Add your license here]

## Contributing

[Add contribution guidelines here]
