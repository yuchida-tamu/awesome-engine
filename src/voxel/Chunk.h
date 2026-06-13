#pragma once

#include <array>
#include <cstdint>

// A Chunk is pure voxel data: a fixed-size 3D grid of block ids.
// It owns NO OpenGL state — turning this data into a mesh is the
// ChunkMesher's job. Keeping storage and meshing separate is what
// lets us unit-test both without a GPU context.
class Chunk {
public:
  // Cubic chunk for now. SIZE^3 cells stored in a flat array.
  static constexpr int SIZE = 16;
  static constexpr int VOLUME = SIZE * SIZE * SIZE;

  // Block id 0 is empty space. Any non-zero id is a solid block (for now).
  static constexpr uint8_t AIR = 0;

  // Read/write a single cell. Coordinates are local to the chunk: [0, SIZE).
  uint8_t BlockAt(int x, int y, int z) const;
  void SetBlock(int x, int y, int z, uint8_t id);

#ifdef UNIT_TEST
public:
#else
private:
#endif
  // Flatten a 3D coordinate into an index into m_blocks.
  int index(int x, int y, int z) const;

  // True if (x, y, z) is a valid cell inside this chunk: each in [0, SIZE).
  bool inBounds(int x, int y, int z) const;

private:
  std::array<uint8_t, VOLUME> m_blocks{}; // value-initialized to AIR (0)
};
