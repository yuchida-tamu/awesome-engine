#include "TestHelpers.h"
#include "doctest.h"

#include "voxel/Chunk.h"
#include "voxel/TerrainGenerator.h"

namespace {

// Number of solid blocks stacked contiguously from the floor of a column.
// Returns -1 if the column has a "floating" block (a solid cell with air
// beneath it) — an invalid heightmap fill.
int solidColumnHeight(const Chunk &chunk, int x, int z) {
  int h = 0;
  while (h < Chunk::SIZE && chunk.blockAt(x, h, z) != Chunk::AIR) {
    ++h;
  }
  for (int y = h; y < Chunk::SIZE; ++y) {
    if (chunk.blockAt(x, y, z) != Chunk::AIR) {
      return -1; // gap / floating block above the solid run
    }
  }
  return h;
}

bool chunksIdentical(const Chunk &a, const Chunk &b) {
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int y = 0; y < Chunk::SIZE; ++y)
      for (int x = 0; x < Chunk::SIZE; ++x)
        if (a.blockAt(x, y, z) != b.blockAt(x, y, z))
          return false;
  return true;
}

} // namespace

// ===================================================================
// FILL TESTS
// ===================================================================

TEST_CASE("TerrainGenerator - columns are filled solid from the floor up") {
  TerrainGenerator gen(1337);
  Chunk chunk = gen.generateChunk(0, 0);

  bool floorAlwaysSolid = true; // height >= 1 everywhere, so y=0 is never air
  bool noFloatingBlocks = true; // every column is a contiguous run from y=0
  for (int z = 0; z < Chunk::SIZE; ++z) {
    for (int x = 0; x < Chunk::SIZE; ++x) {
      if (chunk.blockAt(x, 0, z) == Chunk::AIR)
        floorAlwaysSolid = false;
      if (solidColumnHeight(chunk, x, z) < 0)
        noFloatingBlocks = false;
    }
  }
  CHECK(floorAlwaysSolid);
  CHECK(noFloatingBlocks);
}

// ===================================================================
// BOUNDS TESTS
// ===================================================================

TEST_CASE("TerrainGenerator - terrain stays within the chunk height") {
  TerrainGenerator gen(1337);

  // generateChunk fills via setBlock, which asserts on an out-of-range y.
  CHECK_NOTHROW(gen.generateChunk(0, 0));

  // Max height is SIZE-1, so the topmost layer is always left as air.
  Chunk chunk = gen.generateChunk(0, 0);
  bool topLayerIsAir = true;
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int x = 0; x < Chunk::SIZE; ++x)
      if (chunk.blockAt(x, Chunk::SIZE - 1, z) != Chunk::AIR)
        topLayerIsAir = false;
  CHECK(topLayerIsAir);
}

// ===================================================================
// DETERMINISM / WORLD-COORDINATE TESTS
// ===================================================================

TEST_CASE("TerrainGenerator - same seed and coords produce identical terrain") {
  TerrainGenerator a(1337);
  TerrainGenerator b(1337);
  CHECK(chunksIdentical(a.generateChunk(2, 5), b.generateChunk(2, 5)));
}

TEST_CASE("TerrainGenerator - different chunk coords produce different terrain") {
  // If the world offset weren't applied, every chunk would be identical.
  TerrainGenerator gen(1337);
  CHECK_FALSE(chunksIdentical(gen.generateChunk(0, 0), gen.generateChunk(7, 3)));
}

TEST_CASE("TerrainGenerator - different seeds produce different terrain") {
  TerrainGenerator a(1);
  TerrainGenerator b(2);
  CHECK_FALSE(chunksIdentical(a.generateChunk(0, 0), b.generateChunk(0, 0)));
}
