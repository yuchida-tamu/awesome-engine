#include "doctest.h"
#include "TestHelpers.h"

#include "world/Coords.h"
#include "world/Reconcile.h"

#include <algorithm>
#include <unordered_set>
#include <vector>

// ===================================================================
// LOAD BUDGET
// ===================================================================

TEST_CASE("PlanReconcile - loads all missing desired chunks when within budget") {
  std::unordered_set<int64_t> desired{1, 2, 3};
  std::unordered_set<int64_t> loaded{};

  ReconcilePlan plan = PlanReconcile(desired, loaded, 10);

  CHECK(plan.toLoad.size() == 3);
  CHECK(plan.toUnload.empty());
  CHECK(plan.allLoaded == true);
  for (int64_t k : plan.toLoad) {
    CHECK(desired.count(k) == 1); // only ever loads desired chunks
  }
}

TEST_CASE("PlanReconcile - budget caps loads and reports not-fully-loaded") {
  std::unordered_set<int64_t> desired{1, 2, 3, 4, 5};
  std::unordered_set<int64_t> loaded{};

  ReconcilePlan plan = PlanReconcile(desired, loaded, 2);

  CHECK(plan.toLoad.size() == 2);     // capped at the budget
  CHECK(plan.allLoaded == false);     // more desired remain unloaded
  for (int64_t k : plan.toLoad) {
    CHECK(desired.count(k) == 1);
    CHECK(loaded.count(k) == 0);      // and only the missing ones
  }
}

TEST_CASE("PlanReconcile - missing count exactly equal to budget is fully loaded") {
  std::unordered_set<int64_t> desired{1, 2, 3};
  std::unordered_set<int64_t> loaded{};

  ReconcilePlan plan = PlanReconcile(desired, loaded, 3);

  CHECK(plan.toLoad.size() == 3);
  CHECK(plan.allLoaded == true); // everything fits this frame
}

// ===================================================================
// ALREADY-LOADED HANDLING
// ===================================================================

TEST_CASE("PlanReconcile - already-loaded desired chunks are not reloaded") {
  std::unordered_set<int64_t> desired{1, 2, 3};
  std::unordered_set<int64_t> loaded{1, 2};

  ReconcilePlan plan = PlanReconcile(desired, loaded, 10);

  CHECK(plan.toLoad.size() == 1);
  CHECK(plan.toLoad[0] == 3); // only the one missing chunk
  CHECK(plan.toUnload.empty());
  CHECK(plan.allLoaded == true);
}

// ===================================================================
// UNLOAD
// ===================================================================

TEST_CASE("PlanReconcile - loaded chunks no longer desired are unloaded") {
  std::unordered_set<int64_t> desired{1, 2};
  std::unordered_set<int64_t> loaded{1, 2, 3, 4};

  ReconcilePlan plan = PlanReconcile(desired, loaded, 10);

  CHECK(plan.toLoad.empty());
  std::unordered_set<int64_t> unload(plan.toUnload.begin(), plan.toUnload.end());
  CHECK(unload.size() == 2);
  CHECK(unload.count(3) == 1);
  CHECK(unload.count(4) == 1);
  CHECK(plan.allLoaded == true);
}

TEST_CASE("PlanReconcile - empty desired unloads everything loaded") {
  std::unordered_set<int64_t> desired{};
  std::unordered_set<int64_t> loaded{1, 2, 3};

  ReconcilePlan plan = PlanReconcile(desired, loaded, 10);

  CHECK(plan.toLoad.empty());
  CHECK(plan.toUnload.size() == 3);
  CHECK(plan.allLoaded == true);
}

// ===================================================================
// MIXED
// ===================================================================

TEST_CASE("PlanReconcile - loads missing and unloads stale in one plan") {
  std::unordered_set<int64_t> desired{1, 2, 5}; // keep 1,2; want 5
  std::unordered_set<int64_t> loaded{1, 2, 3};  // have 1,2; 3 is stale

  ReconcilePlan plan = PlanReconcile(desired, loaded, 10);

  CHECK(plan.toLoad.size() == 1);
  CHECK(plan.toLoad[0] == 5);
  CHECK(plan.toUnload.size() == 1);
  CHECK(plan.toUnload[0] == 3);
  CHECK(plan.allLoaded == true);
}

// ===================================================================
// LOAD-BEFORE-UNLOAD (L2)
//
// A stale chunk (loaded, no longer desired) must NOT be unloaded until the
// desired chunk(s) covering its world footprint are loaded -- otherwise the old
// chunk vanishes before its replacement arrives, leaving a hole. Coverage
// follows the LOD nesting: a chunk is replaced by its 8 children (one level
// finer) when the view approaches, or by its parent (one level coarser) when
// the view recedes. If nothing desired covers it, there's no hole to avoid, so
// it unloads immediately.
// ===================================================================

namespace {

// The 8 children (one LOD finer) tiling a chunk's world footprint.
std::vector<int64_t> childKeys(int cx, int cy, int cz, int lod) {
  std::vector<int64_t> kids;
  for (int dx = 0; dx < 2; ++dx)
    for (int dy = 0; dy < 2; ++dy)
      for (int dz = 0; dz < 2; ++dz)
        kids.push_back(
            EncodeKey(2 * cx + dx, 2 * cy + dy, 2 * cz + dz, lod - 1));
  return kids;
}

bool contains(const std::vector<int64_t> &v, int64_t k) {
  return std::find(v.begin(), v.end(), k) != v.end();
}

} // namespace

TEST_CASE("PlanReconcile - keeps a stale chunk while its finer replacements are "
          "still loading") {
  int64_t coarse = EncodeKey(1, 0, 1, 1); // being replaced by its lod-0 children
  std::vector<int64_t> kids = childKeys(1, 0, 1, 1);

  std::unordered_set<int64_t> desired(kids.begin(), kids.end());
  std::unordered_set<int64_t> loaded{coarse};
  loaded.insert(kids.begin(), kids.end() - 1); // all children loaded EXCEPT one

  ReconcilePlan plan = PlanReconcile(desired, loaded, 10);

  CHECK(contains(plan.toLoad, kids.back()));   // the missing child is queued
  CHECK_FALSE(contains(plan.toUnload, coarse)); // coarse kept: a child is missing
}

TEST_CASE("PlanReconcile - unloads a stale chunk once all its finer replacements "
          "are loaded") {
  int64_t coarse = EncodeKey(1, 0, 1, 1);
  std::vector<int64_t> kids = childKeys(1, 0, 1, 1);

  std::unordered_set<int64_t> desired(kids.begin(), kids.end());
  std::unordered_set<int64_t> loaded{coarse};
  loaded.insert(kids.begin(), kids.end()); // ALL children loaded

  ReconcilePlan plan = PlanReconcile(desired, loaded, 10);

  CHECK(contains(plan.toUnload, coarse)); // replacement fully present -> safe
}

TEST_CASE("PlanReconcile - keeps a stale chunk while its coarser replacement is "
          "still loading") {
  int64_t fine = EncodeKey(2, 0, 2, 0);   // receding -> replaced by parent
  int64_t parent = EncodeKey(1, 0, 1, 1); // parent covering it (lod 1)

  std::unordered_set<int64_t> desired{parent};
  std::unordered_set<int64_t> loaded{fine}; // parent NOT loaded yet

  ReconcilePlan plan = PlanReconcile(desired, loaded, 10);

  CHECK(contains(plan.toLoad, parent));       // parent queued
  CHECK_FALSE(contains(plan.toUnload, fine)); // fine kept until parent arrives
}

TEST_CASE("PlanReconcile - unloads a stale chunk once its coarser replacement is "
          "loaded") {
  int64_t fine = EncodeKey(2, 0, 2, 0);
  int64_t parent = EncodeKey(1, 0, 1, 1);

  std::unordered_set<int64_t> desired{parent};
  std::unordered_set<int64_t> loaded{fine, parent}; // parent present

  ReconcilePlan plan = PlanReconcile(desired, loaded, 10);

  CHECK(contains(plan.toUnload, fine));
}

TEST_CASE("PlanReconcile - unloads a stale chunk immediately when nothing covers "
          "it") {
  int64_t gone = EncodeKey(100, 0, 100, 0); // far away; no parent/children desired
  int64_t here = EncodeKey(0, 0, 0, 0);

  std::unordered_set<int64_t> desired{here};
  std::unordered_set<int64_t> loaded{gone, here};

  ReconcilePlan plan = PlanReconcile(desired, loaded, 10);

  CHECK(contains(plan.toUnload, gone)); // no replacement to wait for
}
