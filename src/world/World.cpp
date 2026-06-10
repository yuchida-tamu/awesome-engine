#include "world/World.h"

#include "rendering/Shader.h"
#include "scene/GameObject.h"
#include "scene/RenderComponent.h"
#include "scene/Scene.h"
#include "scene/TransformComponent.h"
#include "voxel/Chunk.h"
#include "voxel/TerrainGenerator.h"
#include "voxel/VoxelChunk.h"
#include <memory>
#include <utility>

World::World(int seed) : m_generator(seed) {}

void World::Populate(Scene &scene, Shader &shader, int centerX, int centerZ,
                     int radius) {
  for (int chunkX = centerX - radius; chunkX <= centerX + radius; ++chunkX) {
    for (int chunkZ = centerZ - radius; chunkZ <= centerZ + radius; ++chunkZ) {
      auto chunk = m_generator.generateChunk(chunkX, chunkZ);
      auto chunkObj = std::make_unique<GameObject>();
      chunkObj->AddComponent<TransformComponent>()->SetPosition(
          {chunkX * Chunk::SIZE, 0, chunkZ * Chunk::SIZE});
      chunkObj->AddComponent<RenderComponent>(
          std::make_unique<VoxelChunk>(chunk), &shader);
      scene.AddGameObject(std::move(chunkObj));
    }
  }
}
