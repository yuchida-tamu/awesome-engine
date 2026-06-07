#include "TestHelpers.h"
#include "doctest.h"

#include <stdexcept>

#include "voxel/Chunk.h"

// ===================================================================
// STORAGE TESTS
// ===================================================================

TEST_CASE("Chunk - A new chunk is entirely air") {
  Chunk chunk;
  CHECK(chunk.blockAt(0, 0, 0) == Chunk::AIR);
  CHECK(chunk.blockAt(5, 9, 3) == Chunk::AIR);
  CHECK(chunk.blockAt(Chunk::SIZE - 1, Chunk::SIZE - 1, Chunk::SIZE - 1) ==
        Chunk::AIR);
}

TEST_CASE("Chunk - setBlock stores a value that blockAt reads back") {
  Chunk chunk;
  chunk.setBlock(1, 2, 3, 7);
  CHECK(chunk.blockAt(1, 2, 3) == 7);
}

// This is the real spec for the index() flattening: if x/y/z were swapped
// or collided, writing one cell would corrupt another.
TEST_CASE("Chunk - setBlock only affects the targeted cell") {
  Chunk chunk;
  chunk.setBlock(1, 2, 3, 7);

  CHECK(chunk.blockAt(0, 2, 3) == Chunk::AIR); // neighbor in x
  CHECK(chunk.blockAt(1, 0, 3) == Chunk::AIR); // neighbor in y
  CHECK(chunk.blockAt(1, 2, 4) == Chunk::AIR); // neighbor in z
}

TEST_CASE("Chunk - distinct cells hold independent values") {
  Chunk chunk;
  chunk.setBlock(0, 0, 0, 1);
  chunk.setBlock(15, 15, 15, 2);

  CHECK(chunk.blockAt(0, 0, 0) == 1);
  CHECK(chunk.blockAt(15, 15, 15) == 2);
}

// ===================================================================
// BOUNDS-CHECKING TESTS
// ===================================================================

// Reading outside the chunk is a designed contract, not an error:
// "outside the chunk is air". This lets the mesher ask about a neighbor
// just past the edge and get a sensible answer for a standalone chunk.
TEST_CASE("Chunk - blockAt outside the chunk returns AIR") {
  Chunk chunk;
  chunk.setBlock(0, 0, 0, 5); // ensure we're not just reading default zeros

  SUBCASE("negative coordinates") {
    CHECK(chunk.blockAt(-1, 0, 0) == Chunk::AIR);
    CHECK(chunk.blockAt(0, -1, 0) == Chunk::AIR);
    CHECK(chunk.blockAt(0, 0, -1) == Chunk::AIR);
  }
  SUBCASE("coordinates at or past SIZE") {
    // Valid indices are 0..SIZE-1, so SIZE itself is the first out-of-range
    // one.
    CHECK(chunk.blockAt(Chunk::SIZE, 0, 0) == Chunk::AIR);
    CHECK(chunk.blockAt(0, Chunk::SIZE, 0) == Chunk::AIR);
    CHECK(chunk.blockAt(0, 0, Chunk::SIZE) == Chunk::AIR);
    CHECK(chunk.blockAt(99, 99, 99) == Chunk::AIR);
  }
}

// Writing outside the chunk is a caller bug — fail loudly.
// ENGINE_ASSERT throws std::logic_error under UNIT_TEST builds.
TEST_CASE("Chunk - setBlock outside the chunk asserts") {
  Chunk chunk;

  CHECK_THROWS_AS(chunk.setBlock(-1, 0, 0, 1), std::logic_error);
  CHECK_THROWS_AS(chunk.setBlock(0, Chunk::SIZE, 0, 1), std::logic_error);
  CHECK_THROWS_AS(chunk.setBlock(0, 0, Chunk::SIZE, 1), std::logic_error);

  // A valid write in the same test must NOT throw.
  CHECK_NOTHROW(
      chunk.setBlock(Chunk::SIZE - 1, Chunk::SIZE - 1, Chunk::SIZE - 1, 1));
}
