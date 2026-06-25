#include "doctest.h"
#include "TestHelpers.h"

#include "world/Reconcile.h"

#include <unordered_set>

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
