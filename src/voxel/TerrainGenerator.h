#pragma once

#include "FastNoiseLite.h"
#include "voxel/Chunk.h"

class TerrainGenerator {
public:
  explicit TerrainGenerator(int seed, float frequency = 0.125f);

  Chunk generateChunk(int chunkX, int chunkZ) const;

private:
  FastNoiseLite m_noise;

  uint8_t getBlockIdForDepth(int depth) const;
};
