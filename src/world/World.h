#pragma once

#include "rendering/Shader.h"
#include "scene/GameObject.h"
#include "scene/Scene.h"
#include "voxel/Chunk.h"
#include "voxel/TerrainGenerator.h"
#include <cstddef>
#include <cstdint>
#include <unordered_map>

class World {
public:
  struct Coord {
    int x;
    int z;
  };

  static constexpr int NUM_VERTICAL_CHUNKS =
      (TerrainGenerator::MAX_TERRAIN_HEIGHT + Chunk::SIZE - 1) / Chunk::SIZE;

  World(int seed = 1337);
  void Update(Scene &scene, Shader &shader, int centerX, int centerZ,
              int radius);

  // Debug stats: cheap reads of the current streamed-in state.
  size_t GetChunkCount() const { return m_map.size(); }
  size_t GetQuadCount() const { return m_totalQuads; }

private:
  // True once every in-radius chunk for the current center is loaded. Goes
  // false when the center moves or a frame's load budget is exhausted, so
  // Update keeps working across frames until the region is fully streamed in.
  bool m_fullyLoaded = false;
  Coord m_currentCoord;

  TerrainGenerator m_generator;
  // A loaded chunk: its scene object plus the quad count of its mesh, so the
  // running total can be adjusted on unload without re-inspecting the mesh.
  struct LoadedChunk {
    GameObject *object;
    size_t quadCount;
  };
  std::unordered_map<int64_t, LoadedChunk> m_map;
  size_t m_totalQuads = 0;

  bool isChunkLoaded(int cx, int cy, int cz) const;
  bool isCenterMoved(int cx, int cz) const;
};
