#include "world/World.h"

#include "rendering/Shader.h"
#include "scene/GameObject.h"
#include "scene/RenderComponent.h"
#include "scene/Scene.h"
#include "scene/TransformComponent.h"
#include "voxel/Chunk.h"
#include "voxel/TerrainGenerator.h"
#include "voxel/VoxelChunk.h"
#include "world/Coords.h"
#include <memory>
#include <utility>

World::World(int seed)
    : m_generator(seed), m_isInitialRender(true), m_currentCoord({0, 0}) {}

void World::Update(Scene &scene, Shader &shader, int centerX, int centerZ,
                   int radius) {
  if (!m_isInitialRender && !isCenterMoved(centerX, centerZ)) {
    return;
  }

  for (int chunkX = centerX - radius; chunkX <= centerX + radius; ++chunkX) {
    for (int chunkZ = centerZ - radius; chunkZ <= centerZ + radius; ++chunkZ) {
      if (isChunkLoaded(chunkX, chunkZ)) {
        continue;
      }

      auto chunk = m_generator.GenerateChunk(chunkX, chunkZ);
      auto chunkObj = std::make_unique<GameObject>();

      // Build the mesh first so we can record its quad count in the running
      // total, then hand the VoxelChunk off to the render component.
      auto voxelChunk = std::make_unique<VoxelChunk>(chunk);
      size_t quadCount = voxelChunk->GetQuadCount();
      m_totalQuads += quadCount;
      m_map[EncodeKey(chunkX, chunkZ)] = {chunkObj.get(), quadCount};

      auto transform = chunkObj->AddComponent<TransformComponent>();
      transform->SetPosition(
          {chunkX * CHUNK_WORLD_SIZE, 0, chunkZ * CHUNK_WORLD_SIZE});
      transform->Scale({VOXEL_SCALE, VOXEL_SCALE, VOXEL_SCALE});
      chunkObj->AddComponent<RenderComponent>(std::move(voxelChunk), &shader);
      scene.AddGameObject(std::move(chunkObj));
    }
  }

  for (auto it = m_map.begin(); it != m_map.end();) {
    auto [cx, cz] = DecodeKey(it->first);
    if (IsOutsideOfRadius(cx, cz, centerX, centerZ, radius)) {
      m_totalQuads -= it->second.quadCount;
      scene.RemoveGameObject(it->second.object);
      it = m_map.erase(it);
    } else {
      ++it;
    }
  }

  m_currentCoord = {centerX, centerZ};
  m_isInitialRender = false;
}

bool World::isChunkLoaded(int cx, int cz) const {
  return m_map.count(EncodeKey(cx, cz)) != 0;
}

bool World::isCenterMoved(int cx, int cz) const {
  return m_currentCoord.x != cx || m_currentCoord.z != cz;
}
