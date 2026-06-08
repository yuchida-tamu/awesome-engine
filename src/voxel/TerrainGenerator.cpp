#include "voxel/TerrainGenerator.h"
#include "voxel/Chunk.h"

TerrainGenerator::TerrainGenerator(int seed, float frequency) {
  m_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
  m_noise.SetSeed(seed);
  m_noise.SetFrequency(frequency);
}

// Set block id at each cell of a Chunk that locates at (chunkX, chunkY)
// coordinate in the world space
Chunk TerrainGenerator::generateChunk(int chunkX, int chunkZ) const {
  Chunk chunk;
  // x and z here are the coordinate within the chunk.
  for (int z = 0; z < Chunk::SIZE; ++z) {
    for (int x = 0; x < Chunk::SIZE; ++x) {
      float n = m_noise.GetNoise((float)(chunkX * Chunk::SIZE + x),
                                 (float)(chunkZ * Chunk::SIZE + z));
      // remap the noise to fit in the chunk coordinate
      int height = (int)((n + 1.0f) * 0.5f * (Chunk::SIZE - 2)) + 1;
      for (int y = 0; y < height; ++y) {
        // its block id is currently hard coded
        // but it should be assigned dynamically once we have more advanced
        // terrain generation system
        chunk.setBlock(x, y, z, 1);
      }
    }
  }

  return chunk;
}
