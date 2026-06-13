#include "TestHelpers.h"
#include "doctest.h"

#include "voxel/Block.h"
#include "voxel/Chunk.h"
#include "voxel/TerrainGenerator.h"

namespace {

// Block ids as plain ints so doctest prints them as numbers, not characters.
const int GRASS = static_cast<uint8_t>(BlockType::Grass);
const int DIRT = static_cast<uint8_t>(BlockType::Dirt);
const int STONE = static_cast<uint8_t>(BlockType::Stone);

// Number of solid blocks stacked contiguously from the floor of a column.
// Returns -1 if the column has a "floating" block (a solid cell with air
// beneath it) — an invalid heightmap fill.
int solidColumnHeight(const Chunk &chunk, int x, int z) {
  int h = 0;
  while (h < Chunk::SIZE && chunk.BlockAt(x, h, z) != Chunk::AIR) {
    ++h;
  }
  for (int y = h; y < Chunk::SIZE; ++y) {
    if (chunk.BlockAt(x, y, z) != Chunk::AIR) {
      return -1; // gap / floating block above the solid run
    }
  }
  return h;
}

bool chunksIdentical(const Chunk &a, const Chunk &b) {
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int y = 0; y < Chunk::SIZE; ++y)
      for (int x = 0; x < Chunk::SIZE; ++x)
        if (a.BlockAt(x, y, z) != b.BlockAt(x, y, z))
          return false;
  return true;
}

} // namespace

// ===================================================================
// FILL TESTS
// ===================================================================

TEST_CASE("TerrainGenerator - columns are filled solid from the floor up") {
  TerrainGenerator gen(1337);
  Chunk chunk = gen.GenerateChunk(0, 0);

  bool floorAlwaysSolid = true; // height >= 1 everywhere, so y=0 is never air
  bool noFloatingBlocks = true; // every column is a contiguous run from y=0
  for (int z = 0; z < Chunk::SIZE; ++z) {
    for (int x = 0; x < Chunk::SIZE; ++x) {
      if (chunk.BlockAt(x, 0, z) == Chunk::AIR)
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

  // GenerateChunk fills via SetBlock, which asserts on an out-of-range y.
  CHECK_NOTHROW(gen.GenerateChunk(0, 0));

  // Max height is SIZE-1, so the topmost layer is always left as air.
  Chunk chunk = gen.GenerateChunk(0, 0);
  bool topLayerIsAir = true;
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int x = 0; x < Chunk::SIZE; ++x)
      if (chunk.BlockAt(x, Chunk::SIZE - 1, z) != Chunk::AIR)
        topLayerIsAir = false;
  CHECK(topLayerIsAir);
}

// ===================================================================
// DETERMINISM / WORLD-COORDINATE TESTS
// ===================================================================

TEST_CASE("TerrainGenerator - same seed and coords produce identical terrain") {
  TerrainGenerator a(1337);
  TerrainGenerator b(1337);
  CHECK(chunksIdentical(a.GenerateChunk(2, 5), b.GenerateChunk(2, 5)));
}

TEST_CASE("TerrainGenerator - different chunk coords produce different terrain") {
  // If the world offset weren't applied, every chunk would be identical.
  TerrainGenerator gen(1337);
  CHECK_FALSE(chunksIdentical(gen.GenerateChunk(0, 0), gen.GenerateChunk(7, 3)));
}

TEST_CASE("TerrainGenerator - different seeds produce different terrain") {
  TerrainGenerator a(1);
  TerrainGenerator b(2);
  CHECK_FALSE(chunksIdentical(a.GenerateChunk(0, 0), b.GenerateChunk(0, 0)));
}

// ===================================================================
// DEPTH-LAYERING TESTS (grass surface / dirt / stone deep)
// ===================================================================

TEST_CASE("TerrainGenerator - the surface cell of every column is grass") {
  TerrainGenerator gen(1337);
  Chunk chunk = gen.GenerateChunk(0, 0);

  bool surfaceAlwaysGrass = true;
  for (int z = 0; z < Chunk::SIZE; ++z) {
    for (int x = 0; x < Chunk::SIZE; ++x) {
      int h = solidColumnHeight(chunk, x, z);
      if (h >= 1 && chunk.BlockAt(x, h - 1, z) != GRASS) {
        surfaceAlwaysGrass = false;
      }
    }
  }
  CHECK(surfaceAlwaysGrass);
}

TEST_CASE("TerrainGenerator - columns layer grass -> dirt -> stone with depth") {
  TerrainGenerator gen(1337);
  Chunk chunk = gen.GenerateChunk(0, 0);

  // Use the tallest column so all three layers are present (need height >= 5
  // to reach depth 4, the first stone cell).
  int bx = 0, bz = 0, tallest = 0;
  for (int z = 0; z < Chunk::SIZE; ++z) {
    for (int x = 0; x < Chunk::SIZE; ++x) {
      int h = solidColumnHeight(chunk, x, z);
      if (h > tallest) {
        tallest = h;
        bx = x;
        bz = z;
      }
    }
  }
  REQUIRE(tallest >= 5); // the seed must produce a column tall enough

  int top = tallest - 1;
  CHECK(chunk.BlockAt(bx, top, bz) == GRASS);     // depth 0 (surface)
  CHECK(chunk.BlockAt(bx, top - 1, bz) == DIRT);  // depth 1
  CHECK(chunk.BlockAt(bx, top - 3, bz) == DIRT);  // depth 3 (last dirt)
  CHECK(chunk.BlockAt(bx, top - 4, bz) == STONE); // depth 4 (first stone)
  CHECK(chunk.BlockAt(bx, 0, bz) == STONE);       // bottom is deep -> stone
}
