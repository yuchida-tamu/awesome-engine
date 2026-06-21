#pragma once

#include "voxel/Chunk.h"
#include <cmath>
#include <cstdint>
#include <tuple>

// VOXEL_SCALE comes from voxel/Chunk.h. The world span of one chunk.
inline constexpr float CHUNK_WORLD_SIZE = Chunk::SIZE * VOXEL_SCALE;

// Maps one axis of a world position to the chunk index containing it.
// Chunk c spans the half-open world range [c*SIZE, (c+1)*SIZE).
inline int WorldToChunk(float worldCoord) {
  return (int)std::floor(worldCoord / CHUNK_WORLD_SIZE);
}

// 3D chunk key: pack (cx, cy, cz) into one int64, 21 signed bits and axis
inline int64_t EncodeKey(int chunkX, int chunkY, int chunkZ) {
  constexpr int64_t BITS = 21;
  constexpr int64_t MASK = (int64_t(1) << BITS) - 1; // low 21 bits
  return (int64_t(chunkX & MASK) << (2 * BITS) |
          int64_t(chunkY & MASK) << BITS | int64_t(chunkZ & MASK));
}

// Recovers (cx, cz) from a key produced by EncodeKey.
inline std::tuple<int, int, int> DecodeKey(int64_t key) {
  // return {(int)(key >> 32), (int)(int32_t)(uint32_t)key};
  constexpr int64_t BITS = 21;
  constexpr int64_t MASK = (int64_t(1) << BITS) - 1; // low 21 bits
  auto sext = [](int64_t v) {             // sign-extend a 21-bit field
    int64_t m = int64_t(1) << (BITS - 1); // bit 20 = the field's sign bit
    return (int)((v ^ m) - m);            // standard sign-extension trick
  };

  return {sext((key >> (2 * BITS)) & MASK), sext((key >> BITS) & MASK),
          sext(key & MASK)};
}

// True if (cx, cz) lies outside the square of the given radius around
// (centerX, centerZ) — i.e. beyond [center +/- radius] on either axis.
inline bool IsOutsideOfRadius(int cx, int cz, int centerX, int centerZ,
                              int radius) {
  bool xOutside = (centerX - radius) > cx || (centerX + radius) < cx;
  bool zOutside = (centerZ - radius) > cz || (centerZ + radius) < cz;
  return xOutside || zOutside;
}

inline int CenterAtLevel(int point, int level) {
  return floor(point / (std::pow(2, level)));
}

inline bool CoveredByFiner(int chunkX, int chunkZ, int finerX, int finerZ,
                           int radius) {
  bool isXCovered =
      chunkX * 2 >= finerX - radius && chunkX * 2 + 1 <= finerX + radius;
  bool isZCovered =
      chunkZ * 2 >= finerZ - radius && chunkZ * 2 + 1 <= finerZ + radius;
  return isXCovered && isZCovered;
}

inline int NumVerticalChunks(int height, int level) {
  // here we use bit-shift instead of std::pow(2, n),
  // since std::pow returns double and we have a risk
  // of a rounding issue.
  auto step = Chunk::SIZE * (1 << level);
  return ceil((height + step - 1) / step);
}
