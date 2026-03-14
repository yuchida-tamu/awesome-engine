# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A 3D OpenGL engine built in C++17 as a learning project. Uses GLFW for windowing, GLAD for OpenGL loading, GLM for math, and Assimp for 3D model importing. Built with CMake.

## Build Commands

```bash
make build          # Build only
make run            # Clean rebuild and run
make rebuild        # Rebuild without cleaning and run
make clean          # Remove bin/ directory
make test           # Build and run tests
make test-verbose   # Build and run tests with all assertions + timing
make test-list      # Build and list all test cases
```

## LSP Setup

CMake is configured with `CMAKE_EXPORT_COMPILE_COMMANDS ON` to generate `compile_commands.json` in `build/`. A symlink at the project root points to it for clangd. If LSP breaks after an Xcode/SDK update, do a clean rebuild: `cd build && rm -rf * && cmake .. && make`.

## Dependencies

Install system dependencies via Homebrew:
```bash
brew install glfw assimp
```

Vendor libraries (GLAD, GLM, stb_image, doctest) are included in `vendor/`.

## Include Order

GLAD must be included before GLFW to avoid OpenGL header conflicts. Use a blank line to prevent formatters from reordering:
```cpp
#include <glad/glad.h>

#include <GLFW/glfw3.h>
```

## Testing

Tests use doctest framework. Test files are in `tests/`. The `UNIT_TEST` macro is defined for test builds to expose private members.

```bash
# Run specific test output options
./bin/run-tests -s          # Show all assertions
./bin/run-tests -s -d       # Show with execution time
./bin/run-tests --list-test-cases  # List all tests
```

## Development Workflow

### TDD (Test-Driven Development)

Follow Kent Beck's TDD discipline for all implementation, refactoring, and bug fixes:

1. **Red** — Write a failing test that defines the desired behavior
2. **Green** — Write the minimum code to make the test pass
3. **Refactor** — Clean up the code while keeping tests green

Use the `cpp-unit-tests` skill when writing tests. Skill references:
- `.claude/skills/cpp-unit-tests/SKILL.md` — skill definition and guidelines
- `.claude/skills/cpp-unit-tests/references/examples.md` — test examples and patterns

See `tests/` for existing test files.

### Separation of Changes

Never mix behavioral changes and structural changes in a single task:

- **Behavioral change**: Adding new functionality, fixing a bug, changing what the code *does*
- **Structural change**: Refactoring, renaming, extracting classes/functions, changing how code is *organized*

Do one, verify it works (tests pass), then do the other. This keeps each change safe, reviewable, and easy to revert.
