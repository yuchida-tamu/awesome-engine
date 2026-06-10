#pragma once

#include "rendering/Shader.h"
#include "scene/Scene.h"
#include "voxel/TerrainGenerator.h"

class World {
public:
  World(int seed = 1337);
  void Populate(Scene &scene, Shader &shader, int radius);

private:
  TerrainGenerator m_generator;
};
