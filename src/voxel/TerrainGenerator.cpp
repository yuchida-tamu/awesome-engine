#include "voxel/TerrainGenerator.h"
#include "voxel/Block.h"
#include "voxel/Chunk.h"
#include <cmath>

TerrainGenerator::TerrainGenerator(int seed, float frequency) {
  m_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  m_noise.SetSeed(seed);
  m_noise.SetFrequency(frequency);
  m_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
  m_noise.SetFractalOctaves(5);
  m_noise.SetFractalLacunarity(2.0f); // Default
  m_noise.SetFractalGain(0.5f);       // Default
  m_noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
  m_noise.SetDomainWarpAmp(20.0f);
}

// Fills the Chunk at chunk-grid coordinate (chunkX, chunkZ), sampling noise in
// world space so neighbouring chunks line up.
Chunk TerrainGenerator::GenerateChunk(int chunkX, int chunkY, int chunkZ,
                                      int lod) const {
  Chunk chunk;
  // x and z here are the coordinate within the chunk.
  for (int z = 0; z < Chunk::SIZE; ++z) {
    for (int x = 0; x < Chunk::SIZE; ++x) {
      float worldX = voxelToWorld(chunkX, x, lod);
      float worldZ = voxelToWorld(chunkZ, z, lod);
      m_noise.DomainWarp(worldX, worldZ);
      float n = m_noise.GetNoise(worldX, worldZ);
      // remap the noise to fit in the chunk coordinate
      float e = (n + 1.0f) * 0.5f; // 0..1
      e = std::pow(e, 3.0f);       // flattens lowlands, keeps peaks tall
      int surfaceY = (int)(e * MAX_TERRAIN_HEIGHT);
      for (int y = 0; y < Chunk::SIZE; ++y) {
        int worldY = chunkY * Chunk::SIZE + y; // y cell position in world
        if (worldY < surfaceY) {
          int depth = (surfaceY - 1) - worldY;
          chunk.SetBlock(x, y, z, getBlockIdForDepth(depth));
        }
        // else: leave air
      }
    }
  }

  return chunk;
}

uint8_t TerrainGenerator::getBlockIdForDepth(int depth) {
  if (depth == 0) {
    return static_cast<uint8_t>(BlockType::Grass);
  }

  if (depth <= 3) {
    return static_cast<uint8_t>(BlockType::Dirt);
  }

  return static_cast<uint8_t>(BlockType::Stone);
}

float TerrainGenerator::voxelToWorld(int chunkCoord, int local, int lod) {
  return (chunkCoord * Chunk::SIZE + local) * VoxelSize(lod);
}
