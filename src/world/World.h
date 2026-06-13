#pragma once

#include "rendering/Shader.h"
#include "scene/GameObject.h"
#include "scene/Scene.h"
#include "voxel/TerrainGenerator.h"
#include <cstdint>
#include <unordered_map>

class World {
public:
  struct Coord {
    int x;
    int z;
  };

  World(int seed = 1337);
  void Update(Scene &scene, Shader &shader, int center, int centerZ,
              int radius);

private:
  bool m_isInitialRender;
  Coord m_currentCoord;

  TerrainGenerator m_generator;
  std::unordered_map<int64_t, GameObject *> m_map;

  bool isChunkLoaded(int cx, int cz) const;
  bool isCenterMoved(int cx, int cz) const;
};
