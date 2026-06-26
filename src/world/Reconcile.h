#pragma once

#include "world/Coords.h"
#include <cstdint>
#include <unordered_set>
#include <vector>

// Pure (GL-free) streaming decision for the LOD chunk reconcile. Given the set
// of chunks we *want* loaded (desired) and the set we *currently* have loaded,
// it produces this frame's plan:
//   - toLoad:   desired chunks not yet loaded, capped at `budget` per frame so
//   a
//               boundary crossing spreads its cost over several frames.
//   - toUnload: loaded chunks no longer desired.
//   - allLoaded: true if every desired chunk is loaded once `toLoad` is applied
//               (i.e. nothing got left behind by the budget).
// Keeping this a pure function over key sets means the reconcile logic is
// unit-testable without a Scene/Shader/OpenGL context.
struct ReconcilePlan {
  std::vector<int64_t> toLoad;
  std::vector<int64_t> toUnload;
  bool allLoaded = true;
};

inline ReconcilePlan PlanReconcile(const std::unordered_set<int64_t> &desired,
                                   const std::unordered_set<int64_t> &loaded,
                                   int budget) {
  ReconcilePlan plan;

  // Load missing desired chunks, up to the per-frame budget.
  for (int64_t key : desired) {
    if (loaded.count(key) != 0) {
      continue;
    }
    if (static_cast<int>(plan.toLoad.size()) < budget) {
      plan.toLoad.push_back(key);
    } else {
      // A desired chunk we can't fit this frame -> not fully streamed yet.
      plan.allLoaded = false;
      break;
    }
  }

  // Unload anything loaded that's no longer desired.
  // If its parent is in desired, it can be unloaded only when the parent is
  // loaded Else if any of its child is in desired, it can be unloaded only when
  // all desired children are in loaded Children = the 8 chunks one level finer
  // at (2cx + dx, 2cy + dy, 2cz + dz, lod - 1)
  for (int64_t key : loaded) {
    if (desired.count(key) != 0) {
      continue;
    }
    // if paratent is in desired
    auto [x, y, z, lod] = DecodeKey(key);
    auto parent = EncodeKey(CenterAtLevel(x, 1), CenterAtLevel(y, 1),
                            CenterAtLevel(z, 1), lod + 1);
    if (desired.count(parent) > 0) {
      if (loaded.count(parent) > 0) {
        plan.toUnload.push_back(key);
      }
      continue;
    }

    int childLod = lod - 1;
    bool isAnyChildDesired = false;
    bool isAllChildrenLoaded = true;
    for (auto dx : {0, 1}) {
      for (auto dy : {0, 1}) {
        for (auto dz : {0, 1}) {
          auto child = EncodeKey(2 * x + dx, 2 * y + dy, 2 * z + dz, childLod);
          if (!isAnyChildDesired) {
            isAnyChildDesired = desired.count(child) > 0;
          }
          if (desired.count(child) > 0 && loaded.count(child) == 0) {
            isAllChildrenLoaded = false;
          }
        }
      }
    }

    if (isAnyChildDesired) {
      if (isAllChildrenLoaded) {
        plan.toUnload.push_back(key);
      }
      continue;
    }

    // if neither its parent nor children are desired, it can be unloaded;
    plan.toUnload.push_back(key);
  }

  return plan;
}
