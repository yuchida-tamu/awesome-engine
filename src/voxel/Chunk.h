#pragma once

#include "core/Assert.h"
#include <array>
#include <cstdint>

// A Chunk is pure voxel data: a fixed-size 3D grid of block ids.
// It owns NO OpenGL state — turning this data into a mesh is the
// ChunkMesher's job. Keeping storage and meshing separate is what
// lets us unit-test both without a GPU context.
class Chunk {
public:
  // Cubic chunk for now. SIZE^3 cells stored in a flat array.
  static constexpr int SIZE = 64;
  static constexpr int VOLUME = SIZE * SIZE * SIZE;

  // Block id 0 is empty space. Any non-zero id is a solid block (for now).
  static constexpr uint8_t AIR = 0;

  // Read/write a single cell. Coordinates are local to the chunk: [0, SIZE).
  inline uint8_t BlockAt(int x, int y, int z) const {
    if (!inBounds(x, y, z))
      return AIR; // outside the chunk is treated as empty space
    return m_blocks[index(x, y, z)];
  }
  inline void SetBlock(int x, int y, int z, uint8_t id) {
    ENGINE_ASSERT(inBounds(x, y, z), "Chunk::SetBlock coordinate out of range");
    m_blocks[index(x, y, z)] = id;
  }

  inline bool IsEmpty() {
    for (uint8_t id : m_blocks) {
      if (id != AIR) {
        return false;
      }
    }
    return true;
  }

#ifdef UNIT_TEST
public:
#else
private:
#endif
  // Flatten a 3D coordinate into an index into m_blocks.
  inline int index(int x, int y, int z) const {
    return x + y * SIZE + z * SIZE * SIZE;
  }

  // True if (x, y, z) is a valid cell inside this chunk: each in [0, SIZE).
  inline bool inBounds(int x, int y, int z) const {
    return x >= 0 && x < SIZE && y >= 0 && y < SIZE && z >= 0 && z < SIZE;
  }

private:
  std::array<uint8_t, VOLUME> m_blocks{}; // value-initialized to AIR (0)
};

// World units per voxel. Lives in the voxel layer (not world/) because terrain
// generation samples noise in world space and must not depend on world/.
// Raising chunk resolution = increase SIZE and decrease VOXEL_SCALE together so
// the chunk's world span (SIZE * VOXEL_SCALE) stays constant.
inline constexpr float VOXEL_SCALE = 0.25f;
