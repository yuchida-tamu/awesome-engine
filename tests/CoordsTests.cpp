#include "TestHelpers.h"
#include "doctest.h"

#include "voxel/Chunk.h"
#include "world/Coords.h"

// worldToChunk maps a world coordinate (one axis) to the index of the chunk
// that contains it. Chunk c spans the half-open range [c*SIZE, (c+1)*SIZE).

// ===================================================================
// POSITIVE / BOUNDARY
// ===================================================================

TEST_CASE("worldToChunk - positions inside the origin chunk map to 0") {
  CHECK(worldToChunk(0.0f) == 0);
  CHECK(worldToChunk(1.0f) == 0);
  CHECK(worldToChunk((float)(Chunk::SIZE - 1)) == 0); // 15 -> 0
}

TEST_CASE("worldToChunk - a chunk boundary belongs to the chunk it starts") {
  CHECK(worldToChunk((float)Chunk::SIZE) == 1);       // 16 -> chunk 1
  CHECK(worldToChunk((float)(2 * Chunk::SIZE)) == 2); // 32 -> chunk 2
}

TEST_CASE("worldToChunk - positive positions floor toward the chunk start") {
  CHECK(worldToChunk(40.0f) == 2); // 40/16 = 2.5 -> chunk 2
  CHECK(worldToChunk(31.9f) == 1); // just below 32 -> chunk 1
}

// ===================================================================
// NEGATIVE (the floor-vs-truncation footgun)
// ===================================================================

// Chunk -1 spans [-16, 0), so any negative-but-near-origin position lives in
// chunk -1, not 0. A naive (int)(world / SIZE) truncates toward zero and
// wrongly returns 0 here.
TEST_CASE("worldToChunk - negative positions floor toward negative infinity") {
  CHECK(worldToChunk(-1.0f) == -1);
  CHECK(worldToChunk(-0.5f) == -1);
  CHECK(worldToChunk((float)(-Chunk::SIZE)) == -1);        // -16 -> chunk -1
  CHECK(worldToChunk((float)(-Chunk::SIZE) - 1.0f) == -2); // -17 -> chunk -2
}

// ===================================================================
// KEY ENCODE / DECODE
// ===================================================================

TEST_CASE("encodeKey/decodeKey round-trips a chunk coordinate") {
  SUBCASE("positive coords") {
    auto [x, z] = decodeKey(encodeKey(3, 7));
    CHECK(x == 3);
    CHECK(z == 7);
  }
  SUBCASE("negative coords survive the round-trip (no sign-extension bleed)") {
    auto [x, z] = decodeKey(encodeKey(-3, 7));
    CHECK(x == -3);
    CHECK(z == 7);

    auto [x2, z2] = decodeKey(encodeKey(-3, -7));
    CHECK(x2 == -3);
    CHECK(z2 == -7);
  }
}

TEST_CASE("encodeKey - distinct coords produce distinct keys") {
  // Not symmetric: (1,2) and (2,1) must differ, and a negative z in one slot
  // must not collide with the same magnitude in the other slot.
  CHECK(encodeKey(1, 2) != encodeKey(2, 1));
  CHECK(encodeKey(-1, 0) != encodeKey(0, -1));
}

// ===================================================================
// RADIUS MEMBERSHIP
// ===================================================================

TEST_CASE("isOutsideOfRadius - coords within the square are NOT outside") {
  CHECK_FALSE(isOutsideOfRadius(0, 0, 0, 0, 1));   // the center itself
  CHECK_FALSE(isOutsideOfRadius(1, 1, 0, 0, 1));   // corner of a radius-1 square
  CHECK_FALSE(isOutsideOfRadius(-1, -1, 0, 0, 1)); // opposite corner
}

TEST_CASE("isOutsideOfRadius - one step past the edge is outside") {
  CHECK(isOutsideOfRadius(2, 0, 0, 0, 1));  // one past +x edge
  CHECK(isOutsideOfRadius(0, -2, 0, 0, 1)); // one past -z edge
}

TEST_CASE("isOutsideOfRadius - works for an off-origin center") {
  CHECK_FALSE(isOutsideOfRadius(5, 5, 5, 5, 2)); // center
  CHECK_FALSE(isOutsideOfRadius(7, 3, 5, 5, 2)); // +x edge / -z edge (inclusive)
  CHECK(isOutsideOfRadius(8, 5, 5, 5, 2));       // one past +x edge
}
