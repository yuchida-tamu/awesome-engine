#pragma once

#include "FastNoiseLite.h"
#include "voxel/Chunk.h"

class TerrainGenerator {
public:
  static constexpr int MAX_TERRAIN_HEIGHT = 256; // in voxel
  explicit TerrainGenerator(int seed, float frequency = 0.015f);

  Chunk GenerateChunk(int chunkX, int chunkY, int chunkZ) const;

private:
  FastNoiseLite m_noise;

#ifdef UNIT_TEST
public:
#else
private:
#endif
  static uint8_t getBlockIdForDepth(int depth);

  static float voxelToWorld(int chunkCoord, int local);
};
