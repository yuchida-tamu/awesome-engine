#include "TestHelpers.h"
#include "doctest.h"

#include "voxel/Block.h"
#include "voxel/Chunk.h"
#include "voxel/TerrainGenerator.h"

// These tests assume the vertical-chunking model:
//   - GenerateChunk(cx, cy, cz) fills the world-Y slice [cy*SIZE, (cy+1)*SIZE).
//   - The surface height of a column is a WORLD value (in voxels), independent
//     of cy, and may exceed Chunk::SIZE (capped at TerrainGenerator::
//     MAX_TERRAIN_HEIGHT).
//   - A voxel is solid iff its worldY < surfaceY.
//
// Requires TerrainGenerator::MAX_TERRAIN_HEIGHT to be a public constant.

namespace {

const int GRASS = static_cast<uint8_t>(BlockType::Grass);
const int DIRT = static_cast<uint8_t>(BlockType::Dirt);
const int STONE = static_cast<uint8_t>(BlockType::Stone);

bool chunksIdentical(const Chunk &a, const Chunk &b) {
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int y = 0; y < Chunk::SIZE; ++y)
      for (int x = 0; x < Chunk::SIZE; ++x)
        if (a.BlockAt(x, y, z) != b.BlockAt(x, y, z))
          return false;
  return true;
}

int solidCount(const Chunk &chunk) {
  int count = 0;
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int y = 0; y < Chunk::SIZE; ++y)
      for (int x = 0; x < Chunk::SIZE; ++x)
        if (chunk.BlockAt(x, y, z) != Chunk::AIR)
          ++count;
  return count;
}

// Within one chunk column: once air appears going up, everything above it must
// also be air (terrain is a solid base with air on top — no floating blocks).
bool columnHasNoFloating(const Chunk &chunk, int x, int z) {
  bool airSeen = false;
  for (int y = 0; y < Chunk::SIZE; ++y) {
    bool solid = chunk.BlockAt(x, y, z) != Chunk::AIR;
    if (solid && airSeen)
      return false;
    if (!solid)
      airSeen = true;
  }
  return true;
}

} // namespace

// ===================================================================
// DETERMINISM / WORLD-COORDINATE TESTS
// ===================================================================

TEST_CASE("TerrainGenerator - same seed and coords produce identical terrain") {
  TerrainGenerator a(1337);
  TerrainGenerator b(1337);
  CHECK(chunksIdentical(a.GenerateChunk(2, 1, 5, 0), b.GenerateChunk(2, 1, 5, 0)));
}

TEST_CASE("TerrainGenerator - different horizontal coords differ") {
  // If the world offset weren't applied in x/z, every column would be the same.
  TerrainGenerator gen(1337);
  CHECK_FALSE(
      chunksIdentical(gen.GenerateChunk(0, 0, 0, 0), gen.GenerateChunk(7, 0, 3, 0)));
}

TEST_CASE("TerrainGenerator - different seeds produce different terrain") {
  TerrainGenerator a(1);
  TerrainGenerator b(2);
  CHECK_FALSE(
      chunksIdentical(a.GenerateChunk(0, 0, 0, 0), b.GenerateChunk(0, 0, 0, 0)));
}

// ===================================================================
// WORLD-HEIGHT DERIVATION
// ===================================================================

TEST_CASE("TerrainGenerator - terrain height is derived from a fixed world height") {
  // MAX_TERRAIN_HEIGHT is a voxel count, but it must represent WORLD_HEIGHT
  // meters regardless of VOXEL_SCALE, so terrain depth stays constant as voxel
  // resolution changes (e.g. 25cm -> 10cm). Equal to within one voxel (floor).
  float derivedWorldHeight = TerrainGenerator::MAX_TERRAIN_HEIGHT * VOXEL_SCALE;
  CHECK(derivedWorldHeight <= TerrainGenerator::WORLD_HEIGHT);
  CHECK(derivedWorldHeight > TerrainGenerator::WORLD_HEIGHT - VOXEL_SCALE);
}

// ===================================================================
// VERTICAL STRUCTURE TESTS
// ===================================================================

TEST_CASE("TerrainGenerator - a chunk below the surface is entirely solid") {
  TerrainGenerator gen(1337);
  // worldY in [-SIZE, 0) is below every surface (surface >= 0), so all solid.
  Chunk below = gen.GenerateChunk(0, -1, 0, 0);
  CHECK(solidCount(below) == Chunk::VOLUME);
}

TEST_CASE("TerrainGenerator - a chunk above the max height is entirely air") {
  TerrainGenerator gen(1337);
  // Lowest worldY of this chunk is >= MAX_TERRAIN_HEIGHT, above every surface.
  int cy = (TerrainGenerator::MAX_TERRAIN_HEIGHT + Chunk::SIZE - 1) / Chunk::SIZE;
  Chunk above = gen.GenerateChunk(0, cy, 0, 0);
  CHECK(solidCount(above) == 0);
}

TEST_CASE("TerrainGenerator - solid voxel count is non-increasing with height") {
  // For a fixed column stack, lower chunks hold at least as much solid as higher
  // ones (the surface is a single threshold).
  TerrainGenerator gen(1337);
  int below = solidCount(gen.GenerateChunk(0, -1, 0, 0));
  int mid = solidCount(gen.GenerateChunk(0, 0, 0, 0));
  int high = solidCount(gen.GenerateChunk(0, 1, 0, 0));
  CHECK(below >= mid);
  CHECK(mid >= high);
}

// ===================================================================
// VERTICAL CONTINUITY (surface independent of chunkY)
// ===================================================================

TEST_CASE("TerrainGenerator - no floating blocks within a chunk") {
  TerrainGenerator gen(1337);
  Chunk chunk = gen.GenerateChunk(0, 0, 0, 0);
  bool ok = true;
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int x = 0; x < Chunk::SIZE; ++x)
      if (!columnHasNoFloating(chunk, x, z))
        ok = false;
  CHECK(ok);
}

TEST_CASE("TerrainGenerator - stacked chunks join without a seam") {
  // The surface for a column must not depend on which cy materializes it:
  // stacking chunk (0,0,0) under (0,1,0) must yield one contiguous solid run
  // (no gap or overlap at the chunk boundary).
  TerrainGenerator gen(1337);
  Chunk c0 = gen.GenerateChunk(0, 0, 0, 0);
  Chunk c1 = gen.GenerateChunk(0, 1, 0, 0);

  bool ok = true;
  for (int z = 0; z < Chunk::SIZE; ++z) {
    for (int x = 0; x < Chunk::SIZE; ++x) {
      bool airSeen = false;
      for (int worldY = 0; worldY < 2 * Chunk::SIZE; ++worldY) {
        bool solid = (worldY < Chunk::SIZE)
                         ? c0.BlockAt(x, worldY, z) != Chunk::AIR
                         : c1.BlockAt(x, worldY - Chunk::SIZE, z) != Chunk::AIR;
        if (solid && airSeen)
          ok = false;
        if (!solid)
          airSeen = true;
      }
    }
  }
  CHECK(ok);
}

// ===================================================================
// DEPTH LAYERING (grass surface / dirt / stone deep)
// ===================================================================

TEST_CASE("TerrainGenerator - surface is grass over dirt over stone") {
  TerrainGenerator gen(1337);
  Chunk chunk = gen.GenerateChunk(0, 0, 0, 0);

  // Find a column whose surface falls *inside* this bottom chunk (solid from
  // y=0 up to some height < SIZE) and is deep enough to expose stone. For such
  // a column the top solid cell is the world surface.
  int sx = -1, sz = -1, height = 0;
  for (int z = 0; z < Chunk::SIZE && sx < 0; ++z) {
    for (int x = 0; x < Chunk::SIZE; ++x) {
      int run = 0;
      while (run < Chunk::SIZE && chunk.BlockAt(x, run, z) != Chunk::AIR)
        ++run;
      if (run >= 5 && run < Chunk::SIZE) {
        sx = x;
        sz = z;
        height = run;
        break;
      }
    }
  }
  REQUIRE(sx >= 0); // the seed must produce a column that crests inside chunk 0

  int top = height - 1;
  CHECK(chunk.BlockAt(sx, top, sz) == GRASS);     // depth 0 (surface)
  CHECK(chunk.BlockAt(sx, top - 1, sz) == DIRT);  // depth 1
  CHECK(chunk.BlockAt(sx, top - 3, sz) == DIRT);  // depth 3 (last dirt)
  CHECK(chunk.BlockAt(sx, top - 4, sz) == STONE); // depth 4 (first stone)
}

// ===================================================================
// VOXEL -> WORLD MAPPING (horizontal cross-chunk continuity)
// ===================================================================

TEST_CASE("TerrainGenerator - voxelToWorld scales voxel indices to world units") {
  CHECK(TerrainGenerator::voxelToWorld(0, 0, 0) == doctest::Approx(0.0f));
  CHECK(TerrainGenerator::voxelToWorld(0, 4, 0) ==
        doctest::Approx(4 * VOXEL_SCALE));
  // Chunk N begins SIZE voxels (in world units) further along.
  CHECK(TerrainGenerator::voxelToWorld(1, 0, 0) ==
        doctest::Approx(Chunk::SIZE * VOXEL_SCALE));
}

TEST_CASE("TerrainGenerator - voxelToWorld is continuous across chunk borders") {
  // The last voxel of one chunk and the first of the next must be exactly one
  // voxel-step apart in world space, so terrain tiles seamlessly. (The earlier
  // `chunkX*VOXEL_SCALE + x` bug made this gap huge.)
  CHECK(TerrainGenerator::voxelToWorld(1, 0, 0) -
            TerrainGenerator::voxelToWorld(0, Chunk::SIZE - 1, 0) ==
        doctest::Approx(VOXEL_SCALE));
  // Same across a negative border (chunk -1 -> chunk 0).
  CHECK(TerrainGenerator::voxelToWorld(0, 0, 0) -
            TerrainGenerator::voxelToWorld(-1, Chunk::SIZE - 1, 0) ==
        doctest::Approx(VOXEL_SCALE));
}

TEST_CASE("TerrainGenerator - voxelToWorld scales with LOD (coarser = bigger)") {
  // A level-1 voxel spans twice the world distance of a level-0 voxel; level 2,
  // four times. This is what makes coarse chunks cover more world.
  CHECK(TerrainGenerator::voxelToWorld(0, 4, 1) ==
        doctest::Approx(2 * TerrainGenerator::voxelToWorld(0, 4, 0)));
  CHECK(TerrainGenerator::voxelToWorld(1, 0, 2) ==
        doctest::Approx(Chunk::SIZE * VOXEL_SCALE * 4));
}
