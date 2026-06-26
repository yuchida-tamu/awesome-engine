#include "TestHelpers.h"
#include "doctest.h"

#include "world/Coords.h" // ComputeDesired + the LOD/key helpers

#include <tuple>
#include <unordered_set>
#include <vector>

// ===================================================================
// DESIRED-SET TILING INVARIANT
//
// The LOD rings must tile the world: every covered point belongs to EXACTLY
// ONE level. More than one => overlap (two LODs render the same terrain ->
// Z-fighting bands). Zero => a gap (a hole). Both are bugs; "exactly one"
// catches them together.
//
// The footprint of chunk (cx,cz,lod) in world-XZ is
//   X: [cx*ChunkSpan(lod), (cx+1)*ChunkSpan(lod)),  Z likewise.
// We sample at lod-0 chunk *centers*: (cx+0.5)*ChunkSpan(0) never lands on a
// coarser chunk boundary, so containment is unambiguous.
// ===================================================================

namespace {

struct Footprint {
  int cx, cz, lod;
};

bool covers(const Footprint &f, float x, float z) {
  float span = ChunkSpan(f.lod);
  float minX = f.cx * span, maxX = (f.cx + 1) * span;
  float minZ = f.cz * span, maxZ = (f.cz + 1) * span;
  return x >= minX && x < maxX && z >= minZ && z < maxZ;
}

// Collapse the vertical (cy) stack: the tiling bug is purely in XZ, so dedup to
// unique (cx, cz, lod) footprints.
std::vector<Footprint> footprints(const std::unordered_set<int64_t> &desired) {
  std::unordered_set<int64_t> seen;
  std::vector<Footprint> out;
  for (int64_t key : desired) {
    auto [cx, cy, cz, lod] = DecodeKey(key);
    (void)cy;
    if (seen.insert(EncodeKey(cx, 0, cz, lod)).second)
      out.push_back({cx, cz, lod});
  }
  return out;
}

int coveringCount(const std::vector<Footprint> &fps, float x, float z) {
  int n = 0;
  for (const auto &f : fps)
    if (covers(f, x, z))
      ++n;
  return n;
}

} // namespace

TEST_CASE("ComputeDesired - exactly one LOD covers each interior point (no "
          "overlap, no gap)") {
  const int centerX = 0, centerZ = 0, radius = 2, maxLod = 3;
  const int maxTerrainHeight = 256; // vertical doesn't affect XZ tiling

  auto desired =
      ComputeDesired(centerX, centerZ, radius, maxLod, maxTerrainHeight);
  auto fps = footprints(desired);

  // Sample lod-0 chunk centers across the fine-ring interior -- the region that
  // must be covered, and where the coarse/fine overlap shows up.
  float span0 = ChunkSpan(0);
  for (int cx = -radius; cx <= radius; ++cx) {
    for (int cz = -radius; cz <= radius; ++cz) {
      float px = (cx + 0.5f) * span0;
      float pz = (cz + 0.5f) * span0;
      CHECK(coveringCount(fps, px, pz) == 1); // not 2 (overlap), not 0 (gap)
    }
  }
}

TEST_CASE("ComputeDesired - center is covered by the finest level only") {
  auto desired = ComputeDesired(0, 0, 2, 3, 256);
  auto fps = footprints(desired);
  // The point at the center sits in lod 0 and must be covered once, by lod 0.
  float span0 = ChunkSpan(0);
  float px = 0.5f * span0, pz = 0.5f * span0;
  int lodCovering = -1, n = 0;
  for (const auto &f : fps)
    if (covers(f, px, pz)) {
      ++n;
      lodCovering = f.lod;
    }
  CHECK(n == 1);
  CHECK(lodCovering == 0);
}
