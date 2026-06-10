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
