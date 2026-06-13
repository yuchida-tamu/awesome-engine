#include "TestHelpers.h"
#include "doctest.h"
#include "voxel/Block.h"

// ===================================================================
// COLOR CHECK TESTS
// ===================================================================

TEST_CASE("Block - returns a Color value based on the block type") {
  CHECK(Vec3ApproxEquals(BlockColor(0u), glm::vec3(1.0, 1.0, 1.0)));
  CHECK(Vec3ApproxEquals(BlockColor(1u), glm::vec3(0.0, 1.0, 0.0)));
  CHECK(Vec3ApproxEquals(BlockColor(2u), glm::vec3(0.5, 0.5, 0.5)));
}
