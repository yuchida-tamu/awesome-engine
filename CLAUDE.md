# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A 3D OpenGL engine built in C++17 as a learning project. Uses GLFW for windowing, GLAD for OpenGL loading, GLM for math, and Assimp for 3D model importing. Built with CMake.

It is being specialized into a **voxel engine** — the foundation for a first-person experience with small (~10 cm) voxels over a large, finite, procedurally-generated world. Rendering is mesh-based with level-of-detail streaming; the world is procedural-on-demand + streaming + LOD, since the full ~10 cm × ~1 km volume can't be materialized in memory. See `README.md` for the roadmap.

## Architecture (voxel pipeline)

- **`src/voxel/`** — pure, GL-free leaf (unit-testable headless):
  - `Chunk` — voxel data, `std::array<uint8_t, SIZE³>` (SIZE = 64); `Air` / `Solid`.
  - `TerrainGenerator` — world-space noise → heightmap; 3D `GenerateChunk(cx, cy, cz, lod)`; runtime-tunable via `UpdateConfig`.
  - `ChunkMesher` (simple per-face) and `GreedyMesher` (coplanar merge) — `Build(Chunk) → MeshData`.
  - `Block` — `Air` / `Solid` + `GetColorByLOD` (debug palette).
  - `VoxelChunk` — a `Drawable`; owns a `Mesh` and sets its own color uniform in `Draw`.
- **`src/world/`** — orchestration (may depend on `voxel/` and `scene/`):
  - `World` — camera-centered chunk streaming.
  - `Coords.h` — pure LOD/key math: `EncodeKey` / `DecodeKey`, `CenterAtLevel`, `CoveredByFiner`, `NumVerticalChunks`, and `ComputeDesired` (the LOD desired-set / ring tiling).
  - `Reconcile.h` — `PlanReconcile`, the pure load/unload decision (load-before-unload).

Two invariants worth protecting, both with tests:
- **LOD rings tile** — exactly one LOD covers any region (no overlap, no gaps): `tests/DesiredSetTests.cpp`.
- **Load-before-unload** — a stale chunk isn't unloaded until its covering replacement (parent or all children) is loaded, so transitions never show holes: `tests/ReconcileTests.cpp`.

The real layering boundary is the `#include` graph, not the folders: keep generation/meshing scene- and GL-free so it stays unit-testable.

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

### Division of Labor

This is a learning project. **The repository owner writes the production code; Claude writes the tests, reviews, and explanations.** In the TDD loop, Claude writes the failing (red) test and explains the approach and trade-offs; the owner implements the production change to make it green; Claude reviews. Do not implement features by editing production source — write/adjust tests and explain. (Test scaffolding — test files and their CMake entries — is Claude's to author.)

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
