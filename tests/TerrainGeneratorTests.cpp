#include "TestHelpers.h"
#include "doctest.h"

#include "voxel/Block.h"
#include "voxel/Chunk.h"
#include "voxel/TerrainGenerator.h"

#include <cmath>
#include <cstdint>

// These tests assume the vertical-chunking model:
//   - GenerateChunk(cx, cy, cz) fills the world-Y slice [cy*SIZE, (cy+1)*SIZE).
//   - The surface height of a column is a WORLD value (in voxels), independent
//     of cy, and may exceed Chunk::SIZE (capped at TerrainGenerator::
//     MAX_TERRAIN_HEIGHT).
//   - A voxel is solid iff its worldY < surfaceY.
//
// Requires TerrainGenerator::MAX_TERRAIN_HEIGHT to be a public constant.

namespace {

bool chunksIdentical(const Chunk &a, const Chunk &b) {
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int y = 0; y < Chunk::SIZE; ++y)
      for (int x = 0; x < Chunk::SIZE; ++x)
        if (a.BlockAt(x, y, z) != b.BlockAt(x, y, z))
          return false;
  return true;
}

// Topmost solid voxel (world-Y) for one column, scanning the full vertical
// stack from the top down. Surface-relative tests use this instead of poking a
// fixed chunk, since which chunk holds the surface depends on WORLD_HEIGHT (at
// a tall world the bottom chunk is just bedrock and is identical everywhere).
int surfaceHeightAt(const TerrainGenerator &gen, int chunkX, int chunkZ,
                    int localX, int localZ) {
  int stack =
      (TerrainGenerator::MAX_TERRAIN_HEIGHT + Chunk::SIZE - 1) / Chunk::SIZE;
  for (int cy = stack - 1; cy >= 0; --cy) {
    Chunk c = gen.GenerateChunk(chunkX, cy, chunkZ, 0);
    for (int y = Chunk::SIZE - 1; y >= 0; --y)
      if (c.BlockAt(localX, y, localZ) != Chunk::AIR)
        return cy * Chunk::SIZE + y;
  }
  return -1;
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
  CHECK(chunksIdentical(a.GenerateChunk(2, 1, 5, 0),
                        b.GenerateChunk(2, 1, 5, 0)));
}

TEST_CASE("TerrainGenerator - different horizontal coords differ") {
  // If the world offset weren't applied in x/z, every column would share a
  // surface height. Two well-separated columns must crest at different heights.
  TerrainGenerator gen(1337);
  CHECK(surfaceHeightAt(gen, 0, 0, 0, 0) != surfaceHeightAt(gen, 7, 3, 0, 0));
}

TEST_CASE("TerrainGenerator - different seeds produce different terrain") {
  TerrainGenerator a(1);
  TerrainGenerator b(2);
  // Surface heights can coincide at a single column, so require a difference
  // somewhere across a short scan of columns.
  bool differs = false;
  for (int cx = 0; cx < 8 && !differs; ++cx) {
    if (surfaceHeightAt(a, cx, 0, 0, 0) != surfaceHeightAt(b, cx, 0, 0, 0))
      differs = true;
  }
  CHECK(differs);
}

// ===================================================================
// WORLD-HEIGHT DERIVATION
// ===================================================================

TEST_CASE(
    "TerrainGenerator - terrain height is derived from a fixed world height") {
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
  int cy =
      (TerrainGenerator::MAX_TERRAIN_HEIGHT + Chunk::SIZE - 1) / Chunk::SIZE;
  Chunk above = gen.GenerateChunk(0, cy, 0, 0);
  CHECK(solidCount(above) == 0);
}

TEST_CASE(
    "TerrainGenerator - solid voxel count is non-increasing with height") {
  // For a fixed column stack, lower chunks hold at least as much solid as
  // higher ones (the surface is a single threshold).
  TerrainGenerator gen(1337);
  int below = solidCount(gen.GenerateChunk(0, -1, 0, 0));
  int mid = solidCount(gen.GenerateChunk(0, 0, 0, 0));
  int high = solidCount(gen.GenerateChunk(0, 1, 0, 0));
  CHECK(below >= mid);
  CHECK(mid >= high);
}

TEST_CASE("TerrainGenerator - surface height is LOD-independent (same world "
          "height)") {
  // e=1 is the tallest column. Its world-space height must be ~the same at
  // every LOD so coarse terrain is as tall as fine. It's exact only when
  // MAX_TERRAIN_HEIGHT divides by 2^lod; otherwise the per-LOD voxel cap
  // truncates, so we allow up to one level-L voxel of difference.
  float refHeight = TerrainGenerator::surfaceVoxelY(1.0f, 0) * VoxelSize(0);
  for (int lod = 0; lod <= 3; ++lod) {
    int sv = TerrainGenerator::surfaceVoxelY(1.0f, lod);
    CHECK(sv == TerrainGenerator::MAX_TERRAIN_HEIGHT /
                    (1 << lod)); // level-L voxel cap
    float worldHeight = sv * VoxelSize(lod);
    CHECK(std::abs(worldHeight - refHeight) < VoxelSize(lod));
  }
  // And it scales linearly with e at a fixed LOD.
  CHECK(TerrainGenerator::surfaceVoxelY(0.5f, 0) ==
        TerrainGenerator::MAX_TERRAIN_HEIGHT / 2);
}

TEST_CASE("TerrainGenerator - terrain height is LOD-independent (coarse not "
          "taller)") {
  // The surface caps at the SAME world height at every LOD. In level-L voxels
  // that cap is MAX_TERRAIN_HEIGHT / 2^L, so a chunk whose whole worldY range
  // is above it must be all air. The bug made coarse terrain 2^L times too
  // tall, so these "above the cap" chunks came back solid.
  TerrainGenerator gen(1337);
  for (int lod = 0; lod <= 2; ++lod) {
    int step = Chunk::SIZE * (1 << lod);
    int aboveCy = (TerrainGenerator::MAX_TERRAIN_HEIGHT + step - 1) /
                  step; // first cy past the cap
    bool allAir = true;
    for (int cz = -2; cz <= 2; ++cz)
      for (int cx = -2; cx <= 2; ++cx)
        if (!gen.GenerateChunk(cx, aboveCy, cz, lod).IsEmpty())
          allAir = false;
    CHECK(allAir);
  }
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

  // We need a column whose surface crests *inside* a bottom chunk (solid from
  // y=0 up to some height in [5, SIZE)) so the top solid cell is the world
  // surface and there's depth enough to expose stone. At a fine VOXEL_SCALE a
  // single chunk spans little world (nearly uniform terrain), so no column may
  // crest inside chunk (0,0,0). Scan a grid of bottom chunks until one turns up
  // (low terrain is common, so this hits quickly) and break out as soon as it
  // does.
  bool checked = false;
  for (int cz = -8; cz <= 8 && !checked; ++cz) {
    for (int cx = -8; cx <= 8 && !checked; ++cx) {
      Chunk chunk = gen.GenerateChunk(cx, 0, cz, 0);
      for (int z = 0; z < Chunk::SIZE && !checked; ++z) {
        for (int x = 0; x < Chunk::SIZE && !checked; ++x) {
          int run = 0;
          while (run < Chunk::SIZE && chunk.BlockAt(x, run, z) != Chunk::AIR)
            ++run;
          if (run >= 5 && run < Chunk::SIZE) {
            int top = run - 1;
            CHECK(chunk.BlockAt(x, top, z) ==
                  static_cast<uint8_t>(BlockType::Solid));
            CHECK(chunk.BlockAt(x, top - 1, z) ==
                  static_cast<uint8_t>(BlockType::Solid));
            CHECK(chunk.BlockAt(x, top - 3, z) ==
                  static_cast<uint8_t>(BlockType::Solid));
            CHECK(chunk.BlockAt(x, top - 4, z) ==
                  static_cast<uint8_t>(BlockType::Solid));
            checked = true;
          }
        }
      }
    }
  }
  REQUIRE(checked); // some chunk in the scanned area must crest inside it
}

// ===================================================================
// VOXEL -> WORLD MAPPING (horizontal cross-chunk continuity)
// ===================================================================

TEST_CASE(
    "TerrainGenerator - voxelToWorld scales voxel indices to world units") {
  CHECK(TerrainGenerator::voxelToWorld(0, 0, 0) == doctest::Approx(0.0f));
  CHECK(TerrainGenerator::voxelToWorld(0, 4, 0) ==
        doctest::Approx(4 * VOXEL_SCALE));
  // Chunk N begins SIZE voxels (in world units) further along.
  CHECK(TerrainGenerator::voxelToWorld(1, 0, 0) ==
        doctest::Approx(Chunk::SIZE * VOXEL_SCALE));
}

TEST_CASE(
    "TerrainGenerator - voxelToWorld is continuous across chunk borders") {
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

TEST_CASE(
    "TerrainGenerator - voxelToWorld scales with LOD (coarser = bigger)") {
  // A level-1 voxel spans twice the world distance of a level-0 voxel; level 2,
  // four times. This is what makes coarse chunks cover more world.
  CHECK(TerrainGenerator::voxelToWorld(0, 4, 1) ==
        doctest::Approx(2 * TerrainGenerator::voxelToWorld(0, 4, 0)));
  CHECK(TerrainGenerator::voxelToWorld(1, 0, 2) ==
        doctest::Approx(Chunk::SIZE * VOXEL_SCALE * 4));
}
