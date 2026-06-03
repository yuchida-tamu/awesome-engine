#include "voxel/Chunk.h"

#include "core/Assert.h"

// Flatten (x, y, z) into a single array index. X-major: x is the
// fastest-varying axis, so walking m_blocks linearly steps x first.
int Chunk::index(int x, int y, int z) const {
  return x + y * SIZE + z * SIZE * SIZE;
}

// Valid cells are 0..SIZE-1 on each axis (a half-open range [0, SIZE)).
bool Chunk::inBounds(int x, int y, int z) const {
  return x >= 0 && x < SIZE && y >= 0 && y < SIZE && z >= 0 && z < SIZE;
}

uint8_t Chunk::blockAt(int x, int y, int z) const {
  if (!inBounds(x, y, z))
    return AIR; // outside the chunk is treated as empty space
  return m_blocks[index(x, y, z)];
}

void Chunk::setBlock(int x, int y, int z, uint8_t id) {
  ENGINE_ASSERT(inBounds(x, y, z), "Chunk::setBlock coordinate out of range");
  m_blocks[index(x, y, z)] = id;
}
