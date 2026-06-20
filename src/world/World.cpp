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

World::World(int seed) : m_generator(seed), m_currentCoord({0, 0}) {}

void World::Update(Scene &scene, Shader &shader, int centerX, int centerZ,
                   int radius) {
  // Nothing to do if the center hasn't moved and the region is fully streamed.
  if (m_fullyLoaded && !isCenterMoved(centerX, centerZ)) {
    return;
  }

  // Generate/mesh/upload at most kLoadBudget chunks per call so a boundary
  // crossing spreads its cost over several frames instead of hitching one.
  // Unloading is cheap (no generation) and stays unbudgeted below.
  constexpr int kLoadBudget = 2;
  int loaded = 0;
  bool allLoaded = true;

  for (int chunkX = centerX - radius; chunkX <= centerX + radius && allLoaded;
       ++chunkX) {
    for (int chunkZ = centerZ - radius; chunkZ <= centerZ + radius; ++chunkZ) {
      for (int chunkY = 0; chunkY < NUM_VERTICAL_CHUNKS; ++chunkY) {
        if (isChunkLoaded(chunkX, chunkY, chunkZ)) {
          continue;
        }
        if (loaded >= kLoadBudget) {
          allLoaded = false; // more remain; finish them on later frames
          break;
        }

        auto chunk = m_generator.GenerateChunk(chunkX, chunkY, chunkZ);

        if (chunk.IsEmpty()) {
          m_map[EncodeKey(chunkX, chunkY, chunkZ)] = {nullptr, 0};
          ++loaded;
          continue;
        }

        auto chunkObj = std::make_unique<GameObject>();

        // Build the mesh first so we can record its quad count in the running
        // total, then hand the VoxelChunk off to the render component.
        auto voxelChunk = std::make_unique<VoxelChunk>(chunk);
        size_t quadCount = voxelChunk->GetQuadCount();
        m_totalQuads += quadCount;
        m_map[EncodeKey(chunkX, chunkY, chunkZ)] = {chunkObj.get(), quadCount};

        auto transform = chunkObj->AddComponent<TransformComponent>();
        transform->SetPosition({chunkX * CHUNK_WORLD_SIZE,
                                chunkY * CHUNK_WORLD_SIZE,
                                chunkZ * CHUNK_WORLD_SIZE});
        transform->Scale({VOXEL_SCALE, VOXEL_SCALE, VOXEL_SCALE});
        chunkObj->AddComponent<RenderComponent>(std::move(voxelChunk), &shader);
        scene.AddGameObject(std::move(chunkObj));

        ++loaded;
      }
    }
  }

  for (auto it = m_map.begin(); it != m_map.end();) {
    auto [cx, cy, cz] = DecodeKey(it->first);
    if (IsOutsideOfRadius(cx, cz, centerX, centerZ, radius)) {
      m_totalQuads -= it->second.quadCount;
      if (it->second.object) {
        scene.RemoveGameObject(it->second.object);
      }

      it = m_map.erase(it);
    } else {
      ++it;
    }
  }

  m_currentCoord = {centerX, centerZ};
  m_fullyLoaded = allLoaded;
}

bool World::isChunkLoaded(int cx, int cy, int cz) const {
  return m_map.count(EncodeKey(cx, cy, cz)) != 0;
}

bool World::isCenterMoved(int cx, int cz) const {
  return m_currentCoord.x != cx || m_currentCoord.z != cz;
}
