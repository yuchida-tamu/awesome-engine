#pragma once

#include "FastNoiseLite.h"
#include "voxel/Chunk.h"

class TerrainGenerator {
public:
  static constexpr float WORLD_HEIGHT = 64.0f;
  static constexpr int MAX_TERRAIN_HEIGHT =
      (int)(WORLD_HEIGHT / VOXEL_SCALE); // in voxel
  explicit TerrainGenerator(int seed, float frequency = 0.015f);

  Chunk GenerateChunk(int chunkX, int chunkY, int chunkZ, int lod) const;

private:
  FastNoiseLite m_noise;

#ifdef UNIT_TEST
public:
#else
private:
#endif
  static uint8_t getBlockIdForDepth(int depth);

  static float voxelToWorld(int chunkCoord, int local, int lod);

  // Surface height of a column, in this LOD's voxels (level-L voxels). Caps at
  // MAX_TERRAIN_HEIGHT / 2^lod, which is the same WORLD height at every LOD.
  static int surfaceVoxelY(float e, int lod);
};
