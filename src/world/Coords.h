#pragma once

#include "voxel/Chunk.h"
#include <cmath>
#include <cstdint>
#include <tuple>

// Maps one axis of a world position to the chunk index containing it.
// Chunk c spans the half-open world range [c*SIZE, (c+1)*SIZE).
inline int worldToChunk(float worldCoord) {
  return (int)std::floor(worldCoord / Chunk::SIZE);
}

// Packs a chunk coordinate (cx, cz) into one 64-bit key: cx in the high 32
// bits, cz in the low 32. The (uint32_t) cast stops a negative cz from
// sign-extending into the high half. Reversible and collision-free.
inline int64_t encodeKey(int chunkX, int chunkZ) {
  return ((int64_t)chunkX << 32) | (uint32_t)chunkZ;
}

// Recovers (cx, cz) from a key produced by encodeKey.
inline std::tuple<int, int> decodeKey(int64_t key) {
  return {(int)(key >> 32), (int)(int32_t)(uint32_t)key};
}

// True if (cx, cz) lies outside the square of the given radius around
// (centerX, centerZ) — i.e. beyond [center +/- radius] on either axis.
inline bool isOutsideOfRadius(int cx, int cz, int centerX, int centerZ,
                              int radius) {
  bool xOutside = (centerX - radius) > cx || (centerX + radius) < cx;
  bool zOutside = (centerZ - radius) > cz || (centerZ + radius) < cz;
  return xOutside || zOutside;
}
