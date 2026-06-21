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

  // Phase 2: load missing desired chunkc, at most kLoadBudget per call
  // so a boundary crossing spreads is cost over several frames
  // Loads per frame. Higher = transition strips fill faster (less LOD-change
  // flicker), at the cost of a heavier frame on big boundary crossings. -O3
  // gives us the headroom; 2 was a conservative -O0-era value.
  constexpr int kLoadBudget = 6;
  int loaded = 0;
  bool allLoaded = true;
  for (int64_t key : desired) {
    if (m_map.count(key) != 0) {
      continue; // already loaded
    }
    if (loaded >= kLoadBudget) {
      allLoaded = false;
      break;
    }

    auto [cx, cy, cz, lod] = DecodeKey(key);
    loadChunk(scene, shader, cx, cy, cz, lod);
    ++loaded;
  }

  // Phase 3: unload anything no longer desired
  for (auto it = m_map.begin(); it != m_map.end();) {
    if (desired.count(it->first) == 0) {
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
