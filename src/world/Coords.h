#pragma once

#include "voxel/Chunk.h"
#include <cmath>

// Maps one axis of a world position to the chunk index containing it.
// Chunk c spans the half-open world range [c*SIZE, (c+1)*SIZE).
inline int worldToChunk(float worldCoord) {
  return (int)std::floor(worldCoord / Chunk::SIZE);
}
