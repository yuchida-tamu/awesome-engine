#include "TestHelpers.h"
#include "doctest.h"

#include "voxel/Block.h"
#include "voxel/Chunk.h"
#include "voxel/ChunkMesher.h"
#include "voxel/GreedyMesher.h"

namespace {

// Total surface area of a mesh = sum of triangle areas. Robust: it doesn't
// assume any quad structure, so it compares greedy vs simple meshes fairly.
float surfaceArea(const MeshData &m) {
  float area = 0.0f;
  for (size_t t = 0; t + 2 < m.indices.size(); t += 3) {
    const glm::vec3 &a = m.vertices[m.indices[t + 0]].position;
    const glm::vec3 &b = m.vertices[m.indices[t + 1]].position;
    const glm::vec3 &c = m.vertices[m.indices[t + 2]].position;
    area += 0.5f * glm::length(glm::cross(b - a, c - a));
  }
  return area;
}

// Surface area restricted to triangles whose color matches `color` — lets us
// check per-material coverage (catches a wrong cross-type merge).
float areaOfColor(const MeshData &m, const glm::vec3 &color) {
  float area = 0.0f;
  for (size_t t = 0; t + 2 < m.indices.size(); t += 3) {
    const Mesh::Vertex &v0 = m.vertices[m.indices[t + 0]];
    if (!Vec3ApproxEquals(v0.color, color))
      continue;
    const glm::vec3 &b = m.vertices[m.indices[t + 1]].position;
    const glm::vec3 &c = m.vertices[m.indices[t + 2]].position;
    area += 0.5f * glm::length(glm::cross(b - v0.position, c - v0.position));
  }
  return area;
}

size_t quadCount(const MeshData &m) { return m.indices.size() / 6; }

void fillSolid(Chunk &chunk, uint8_t id) {
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int y = 0; y < Chunk::SIZE; ++y)
      for (int x = 0; x < Chunk::SIZE; ++x)
        chunk.SetBlock(x, y, z, id);
}

} // namespace

// ===================================================================
// EQUIVALENCE WITH THE SIMPLE MESHER (the oracle)
// ===================================================================

TEST_CASE("GreedyMesher - an empty chunk produces no geometry") {
  Chunk chunk;
  MeshData mesh = GreedyMesher::Build(chunk);
  CHECK(mesh.vertices.empty());
  CHECK(mesh.indices.empty());
}

TEST_CASE("GreedyMesher - a single block: 6 quads, same surface as simple") {
  Chunk chunk;
  chunk.SetBlock(0, 0, 0, 1);

  MeshData greedy = GreedyMesher::Build(chunk);
  MeshData simple = ChunkMesher::Build(chunk);

  CHECK(quadCount(greedy) == 6); // nothing to merge -> identical to simple
  CHECK(surfaceArea(greedy) == doctest::Approx(surfaceArea(simple)));
}

// The strongest test: for an arbitrary chunk, greedy must cover EXACTLY the
// same surface as the simple mesher — no faces lost, added, or moved.
TEST_CASE("GreedyMesher - covers the same surface as the simple mesher") {
  Chunk chunk;
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int x = 0; x < Chunk::SIZE; ++x) {
      int h = 1 + (x * 3 + z * 5) % 6;             // varied heights 1..6
      uint8_t id = ((x / 4 + z / 4) % 2) ? 1 : 2;  // blocky material regions
      for (int y = 0; y < h; ++y)
        chunk.SetBlock(x, y, z, id);
    }

  MeshData greedy = GreedyMesher::Build(chunk);
  MeshData simple = ChunkMesher::Build(chunk);

  CHECK(surfaceArea(greedy) == doctest::Approx(surfaceArea(simple)));
}

// ===================================================================
// MERGING ACTUALLY HAPPENS
// ===================================================================

TEST_CASE("GreedyMesher - a solid chunk merges each side into one quad") {
  Chunk chunk;
  fillSolid(chunk, 1);

  MeshData greedy = GreedyMesher::Build(chunk);
  MeshData simple = ChunkMesher::Build(chunk);

  CHECK(quadCount(greedy) == 6); // one big quad per cube side
  CHECK(surfaceArea(greedy) == doctest::Approx(surfaceArea(simple)));
}

TEST_CASE("GreedyMesher - a flat layer needs far fewer quads than simple") {
  Chunk chunk;
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int x = 0; x < Chunk::SIZE; ++x)
      chunk.SetBlock(x, 0, z, 1); // full bottom layer

  MeshData greedy = GreedyMesher::Build(chunk);
  MeshData simple = ChunkMesher::Build(chunk);

  CHECK(quadCount(greedy) < quadCount(simple));
  CHECK(surfaceArea(greedy) == doctest::Approx(surfaceArea(simple)));
}

// ===================================================================
// GEOMETRY CORRECTNESS
// ===================================================================

// Merged quads must still wind so their normal points outward, and the normal
// must be an axis-aligned unit vector. Uses a solid chunk so all six sides are
// large merged quads.
TEST_CASE("GreedyMesher - merged quads wind to outward axis-aligned normals") {
  Chunk chunk;
  fillSolid(chunk, 1);

  MeshData mesh = GreedyMesher::Build(chunk);
  REQUIRE(mesh.indices.size() >= 6);

  for (size_t t = 0; t < mesh.indices.size(); t += 3) {
    const Mesh::Vertex &v0 = mesh.vertices[mesh.indices[t + 0]];
    const Mesh::Vertex &v1 = mesh.vertices[mesh.indices[t + 1]];
    const Mesh::Vertex &v2 = mesh.vertices[mesh.indices[t + 2]];

    glm::vec3 geometricNormal = glm::normalize(
        glm::cross(v1.position - v0.position, v2.position - v0.position));
    CHECK(Vec3ApproxEquals(geometricNormal, v0.normal)); // winding agrees

    // normal is one of the six axis directions (unit length on one axis)
    float mag = std::abs(v0.normal.x) + std::abs(v0.normal.y) + std::abs(v0.normal.z);
    CHECK(mag == doctest::Approx(1.0f));
  }
}

// ===================================================================
// COLORS / MATERIALS NOT MERGED ACROSS TYPES
// ===================================================================

// A flat layer split half grass / half stone: greedy must not merge across the
// material boundary, so per-color coverage stays equal to the simple mesher.
TEST_CASE("GreedyMesher - does not merge across block types") {
  Chunk chunk;
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int x = 0; x < Chunk::SIZE; ++x)
      chunk.SetBlock(x, 0, z, x < Chunk::SIZE / 2 ? 1 : 2);

  MeshData greedy = GreedyMesher::Build(chunk);
  MeshData simple = ChunkMesher::Build(chunk);

  glm::vec3 grass = BlockColor(1);
  glm::vec3 stone = BlockColor(2);
  CHECK(areaOfColor(greedy, grass) == doctest::Approx(areaOfColor(simple, grass)));
  CHECK(areaOfColor(greedy, stone) == doctest::Approx(areaOfColor(simple, stone)));
}
