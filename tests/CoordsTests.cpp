#include "TestHelpers.h"
#include "doctest.h"

#include "voxel/Chunk.h"
#include "world/Coords.h"

// WorldToChunk maps a world coordinate (one axis) to the index of the chunk
// that contains it. A chunk spans the half-open world range
// [c*CHUNK_WORLD_SIZE, (c+1)*CHUNK_WORLD_SIZE) — the chunk's WORLD span
// (SIZE * VOXEL_SCALE), NOT its voxel count (Chunk::SIZE).

// ===================================================================
// POSITIVE / BOUNDARY
// ===================================================================

TEST_CASE("WorldToChunk - positions inside the origin chunk map to 0") {
  CHECK(WorldToChunk(0.0f) == 0);
  CHECK(WorldToChunk(1.0f) == 0);
  CHECK(WorldToChunk(CHUNK_WORLD_SIZE - 1.0f) == 0); // just inside the top edge
}

TEST_CASE("WorldToChunk - a chunk boundary belongs to the chunk it starts") {
  CHECK(WorldToChunk(CHUNK_WORLD_SIZE) == 1);        // first cell of chunk 1
  CHECK(WorldToChunk(2.0f * CHUNK_WORLD_SIZE) == 2); // first cell of chunk 2
}

TEST_CASE("WorldToChunk - positive positions floor toward the chunk start") {
  CHECK(WorldToChunk(2.5f * CHUNK_WORLD_SIZE) == 2);        // mid chunk 2
  CHECK(WorldToChunk(2.0f * CHUNK_WORLD_SIZE - 0.1f) == 1); // just below chunk 2
}

// ===================================================================
// NEGATIVE (the floor-vs-truncation footgun)
// ===================================================================

// Chunk -1 spans [-CHUNK_WORLD_SIZE, 0), so any negative-but-near-origin
// position lives in chunk -1, not 0. A naive (int)(world / size) truncates
// toward zero and wrongly returns 0 here.
TEST_CASE("WorldToChunk - negative positions floor toward negative infinity") {
  CHECK(WorldToChunk(-1.0f) == -1);
  CHECK(WorldToChunk(-0.5f) == -1);
  CHECK(WorldToChunk(-CHUNK_WORLD_SIZE) == -1);        // first cell of chunk -1
  CHECK(WorldToChunk(-CHUNK_WORLD_SIZE - 1.0f) == -2); // chunk -2
}

// ===================================================================
// KEY ENCODE / DECODE
// ===================================================================

TEST_CASE("EncodeKey/DecodeKey round-trips a 3D chunk coordinate") {
  SUBCASE("positive coords") {
    auto [x, y, z] = DecodeKey(EncodeKey(3, 7, 5));
    CHECK(x == 3);
    CHECK(y == 7);
    CHECK(z == 5);
  }
  SUBCASE("negative coords survive the round-trip (sign-extension)") {
    auto [x, y, z] = DecodeKey(EncodeKey(-3, -7, -5));
    CHECK(x == -3);
    CHECK(y == -7);
    CHECK(z == -5);
  }
  SUBCASE("mixed signs don't bleed between axes") {
    auto [x, y, z] = DecodeKey(EncodeKey(-3, 7, -5));
    CHECK(x == -3);
    CHECK(y == 7);
    CHECK(z == -5);
  }
  SUBCASE("multi-bit values within range round-trip") {
    auto [x, y, z] = DecodeKey(EncodeKey(100000, -50000, 12345));
    CHECK(x == 100000);
    CHECK(y == -50000);
    CHECK(z == 12345);
  }
}

TEST_CASE("EncodeKey - distinct coords produce distinct keys") {
  // Each axis must occupy its own field: permutations and per-axis unit steps
  // must all differ, and a negative in one axis must not collide with another.
  CHECK(EncodeKey(1, 2, 3) != EncodeKey(3, 2, 1));
  CHECK(EncodeKey(1, 0, 0) != EncodeKey(0, 1, 0));
  CHECK(EncodeKey(0, 1, 0) != EncodeKey(0, 0, 1));
  CHECK(EncodeKey(-1, 0, 0) != EncodeKey(0, -1, 0));
  CHECK(EncodeKey(-1, 0, 0) != EncodeKey(0, 0, -1));
}

// ===================================================================
// RADIUS MEMBERSHIP
// ===================================================================

TEST_CASE("IsOutsideOfRadius - coords within the square are NOT outside") {
  CHECK_FALSE(IsOutsideOfRadius(0, 0, 0, 0, 1));   // the center itself
  CHECK_FALSE(IsOutsideOfRadius(1, 1, 0, 0, 1));   // corner of a radius-1 square
  CHECK_FALSE(IsOutsideOfRadius(-1, -1, 0, 0, 1)); // opposite corner
}

TEST_CASE("IsOutsideOfRadius - one step past the edge is outside") {
  CHECK(IsOutsideOfRadius(2, 0, 0, 0, 1));  // one past +x edge
  CHECK(IsOutsideOfRadius(0, -2, 0, 0, 1)); // one past -z edge
}

TEST_CASE("IsOutsideOfRadius - works for an off-origin center") {
  CHECK_FALSE(IsOutsideOfRadius(5, 5, 5, 5, 2)); // center
  CHECK_FALSE(IsOutsideOfRadius(7, 3, 5, 5, 2)); // +x edge / -z edge (inclusive)
  CHECK(IsOutsideOfRadius(8, 5, 5, 5, 2));       // one past +x edge
}

// ===================================================================
// LOD MATH (M1)
// ===================================================================

// CenterAtLevel: the camera's chunk coordinate at a coarser level is the
// finer center floor-divided by 2^lod. Floor (not truncation) so the levels
// nest correctly across the origin.
TEST_CASE("CenterAtLevel - lod 0 is the identity") {
  CHECK(CenterAtLevel(0, 0) == 0);
  CHECK(CenterAtLevel(7, 0) == 7);
  CHECK(CenterAtLevel(-7, 0) == -7);
}

TEST_CASE("CenterAtLevel - positive centers halve per level") {
  CHECK(CenterAtLevel(10, 1) == 5);
  CHECK(CenterAtLevel(7, 1) == 3);  // floor(7/2)
  CHECK(CenterAtLevel(10, 2) == 2); // floor(10/4)
}

TEST_CASE("CenterAtLevel - negative centers floor toward -infinity") {
  CHECK(CenterAtLevel(-1, 1) == -1); // floor(-1/2) = -1, NOT 0
  CHECK(CenterAtLevel(-3, 1) == -2); // floor(-3/2) = -2
  CHECK(CenterAtLevel(-1, 2) == -1); // floor(-1/4) = -1
}

TEST_CASE("CenterAtLevel - levels nest (stepping up one == two single steps)") {
  for (int c : {-9, -2, 5, 23}) {
    CHECK(CenterAtLevel(c, 2) == CenterAtLevel(CenterAtLevel(c, 1), 1));
  }
}

// CoveredByFiner: a level-L chunk is in the hole when the 2x2 block of finer
// chunks it covers ([2c, 2c+1] per axis) lies entirely inside the finer level's
// (2R+1) square around the finer center.
TEST_CASE("CoveredByFiner - chunks deep inside the finer square are covered") {
  CHECK(CoveredByFiner(0, 0, 0, 0, 4));   // covers finer [0,1]x[0,1]
  CHECK(CoveredByFiner(1, 1, 0, 0, 4));   // covers finer [2,3]x[2,3]
  CHECK(CoveredByFiner(-2, 0, 0, 0, 4));  // covers finer [-4,-3] — on the edge, still inside
}

TEST_CASE("CoveredByFiner - chunks past the finer square are NOT covered") {
  CHECK_FALSE(CoveredByFiner(2, 2, 0, 0, 4));  // finer [4,5] -> 5 > 4, outside
  CHECK_FALSE(CoveredByFiner(-3, 0, 0, 0, 4)); // finer [-6,-5] -> -6 < -4, outside
}

TEST_CASE("CoveredByFiner - must be inside on BOTH axes") {
  CHECK_FALSE(CoveredByFiner(0, 3, 0, 0, 4)); // x inside, z ([6,7]) outside
}

// NumVerticalChunks: how many chunks tall the terrain is at a given LOD. Coarser
// levels need fewer (each chunk is 2x taller). Using a height of 4*SIZE so the
// expected 4,2,1,1 progression is independent of Chunk::SIZE.
TEST_CASE("NumVerticalChunks - halves per level (with a floor of 1)") {
  const int height = 4 * Chunk::SIZE;
  CHECK(NumVerticalChunks(height, 0) == 4);
  CHECK(NumVerticalChunks(height, 1) == 2);
  CHECK(NumVerticalChunks(height, 2) == 1);
  CHECK(NumVerticalChunks(height, 3) == 1); // ceil(0.5) -> 1, never drops to 0
}
