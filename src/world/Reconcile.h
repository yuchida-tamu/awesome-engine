#pragma once

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
      continue; // already loaded
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
  for (int64_t key : loaded) {
    if (desired.count(key) == 0) {
      plan.toUnload.push_back(key);
    }
  }

  return plan;
}
