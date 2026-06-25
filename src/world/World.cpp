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
#include "world/Reconcile.h"
#include <memory>
#include <unordered_set>
#include <utility>

World::World(int seed) : m_generator(seed), m_currentCoord({0, 0}) {}

void World::Update(Scene &scene, Shader &shader, int centerX, int centerZ,
                   int radius) {
  // Nothing to do if the center hasn't moved and the region is fully streamed.
  if (m_fullyLoaded && !isCenterMoved(centerX, centerZ)) {
    return;
  }

  // Phase 1: enumerate the full desired set across all LOD rings. No budget. No
  // loading
  std::unordered_set<int64_t> desired;
  for (int lod = 0; lod <= MAX_LOD; ++lod) {
    int centerAtLevelX = CenterAtLevel(centerX, lod);
    int centerAtLevelZ = CenterAtLevel(centerZ, lod);
    int finerCenterAtLevelX = CenterAtLevel(centerX, lod - 1);
    int finerCenterAtLevelZ = CenterAtLevel(centerZ, lod - 1);
    int verticalChunks =
        NumVerticalChunks(TerrainGenerator::MAX_TERRAIN_HEIGHT, lod);

    for (int cx = centerAtLevelX - radius; cx <= centerAtLevelX + radius;
         ++cx) {
      for (int cz = centerAtLevelZ - radius; cz <= centerAtLevelZ + radius;
           ++cz) {
        if (lod > 0 && CoveredByFiner(cx, cz, finerCenterAtLevelX,
                                      finerCenterAtLevelZ, radius)) {
          continue;
        }
        for (int cy = 0; cy < verticalChunks; ++cy) {
          desired.insert(EncodeKey(cx, cy, cz, lod));
        }
      }
    }
  }

  // Phase 2: decide what to load/unload. kLoadBudget caps loads per call so a
  // boundary crossing spreads its cost over several frames. Higher = transition
  // strips fill faster (less LOD-change flicker), at the cost of a heavier frame
  // on big boundary crossings. -O3 gives us the headroom; 2 was a conservative
  // -O0-era value. The decision itself is a pure function (PlanReconcile) so it
  // can be unit-tested without a Scene/Shader.
  constexpr int kLoadBudget = 6;
  std::unordered_set<int64_t> loadedKeys;
  loadedKeys.reserve(m_map.size());
  for (const auto &entry : m_map) {
    loadedKeys.insert(entry.first);
  }
  ReconcilePlan plan = PlanReconcile(desired, loadedKeys, kLoadBudget);

  // Phase 3: execute the plan against the scene.
  for (int64_t key : plan.toLoad) {
    auto [cx, cy, cz, lod] = DecodeKey(key);
    loadChunk(scene, shader, cx, cy, cz, lod);
  }
  for (int64_t key : plan.toUnload) {
    auto it = m_map.find(key);
    if (it == m_map.end()) {
      continue;
    }
    m_totalQuads -= it->second.quadCount;
    if (it->second.object) {
      scene.RemoveGameObject(it->second.object);
    }
    m_map.erase(it);
  }

  m_currentCoord = {centerX, centerZ};
  m_fullyLoaded = plan.allLoaded;
}

void World::UpdateTerrainConfig(TerrainGenerator::Parameter param,
                                float value) {
  m_generator.UpdateConfig(param, value);
}

void World::loadChunk(Scene &scene, Shader &shader, int chunkX, int chunkY,
                      int chunkZ, int lod) {
  auto chunk = m_generator.GenerateChunk(chunkX, chunkY, chunkZ, lod);

  if (chunk.IsEmpty()) {
    m_map[EncodeKey(chunkX, chunkY, chunkZ, lod)] = {nullptr, 0};
    return;
  }

  auto chunkObj = std::make_unique<GameObject>();

  // Build the mesh first so we can record its quad count in the running
  // total, then hand the VoxelChunk off to the render component.
  auto voxelChunk = std::make_unique<VoxelChunk>(chunk);
  size_t quadCount = voxelChunk->GetQuadCount();
  m_totalQuads += quadCount;
  m_map[EncodeKey(chunkX, chunkY, chunkZ, lod)] = {chunkObj.get(), quadCount};

  auto transform = chunkObj->AddComponent<TransformComponent>();
  auto chunkSpan = ChunkSpan(lod);
  auto voxelSize = VoxelSize(lod);
  transform->SetPosition(
      {chunkX * chunkSpan, chunkY * chunkSpan, chunkZ * chunkSpan});
  transform->Scale({voxelSize, voxelSize, voxelSize});
  chunkObj->AddComponent<RenderComponent>(std::move(voxelChunk), &shader);
  scene.AddGameObject(std::move(chunkObj));
}

bool World::isChunkLoaded(int cx, int cy, int cz) const {
  return m_map.count(EncodeKey(cx, cy, cz, 0)) != 0;
}

bool World::isCenterMoved(int cx, int cz) const {
  return m_currentCoord.x != cx || m_currentCoord.z != cz;
}
