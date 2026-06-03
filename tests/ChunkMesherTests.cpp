#include "doctest.h"
#include "TestHelpers.h"

#include "voxel/ChunkMesher.h"

// ===================================================================
// FACE-COUNT TESTS
// ===================================================================

TEST_CASE("ChunkMesher - an empty chunk produces no geometry") {
  Chunk chunk;
  MeshData mesh = ChunkMesher::Build(chunk);

  CHECK(mesh.vertices.empty());
  CHECK(mesh.indices.empty());
}

TEST_CASE("ChunkMesher - a single solid block produces 6 faces") {
  Chunk chunk;
  chunk.setBlock(0, 0, 0, 1);

  MeshData mesh = ChunkMesher::Build(chunk);

  CHECK(mesh.vertices.size() == 24); // 6 faces * 4 vertices each
  CHECK(mesh.indices.size() == 36);  // 6 faces * 2 triangles * 3 indices
}

// ===================================================================
// GEOMETRY CORRECTNESS TESTS
// ===================================================================

// For every emitted triangle, the geometric normal derived from its winding
// (cross product of two edges) must agree with the normal we stored on the
// vertices. This catches inside-out winding and mislabeled normals without
// ever rendering a frame.
TEST_CASE("ChunkMesher - triangle winding agrees with stored normals") {
  Chunk chunk;
  chunk.setBlock(0, 0, 0, 1);

  MeshData mesh = ChunkMesher::Build(chunk);
  REQUIRE(mesh.indices.size() == 36);

  for (size_t t = 0; t < mesh.indices.size(); t += 3) {
    const Mesh::Vertex &v0 = mesh.vertices[mesh.indices[t + 0]];
    const Mesh::Vertex &v1 = mesh.vertices[mesh.indices[t + 1]];
    const Mesh::Vertex &v2 = mesh.vertices[mesh.indices[t + 2]];

    glm::vec3 geometricNormal = glm::normalize(
        glm::cross(v1.position - v0.position, v2.position - v0.position));

    // All verts of a face carry the same normal; compare against v0's.
    CHECK(Vec3ApproxEquals(geometricNormal, v0.normal));
  }
}

// A lone block sits in air on all sides, so it must expose all six
// axis-aligned faces — each direction exactly once, no duplicates or gaps.
TEST_CASE("ChunkMesher - a single block exposes all six face directions") {
  Chunk chunk;
  chunk.setBlock(0, 0, 0, 1);

  MeshData mesh = ChunkMesher::Build(chunk);
  REQUIRE(mesh.vertices.size() == 24); // 6 faces * 4 verts

  const glm::vec3 axes[6] = {{1, 0, 0},  {-1, 0, 0}, {0, 1, 0},
                             {0, -1, 0}, {0, 0, 1},  {0, 0, -1}};

  for (const glm::vec3 &axis : axes) {
    int facesWithThisNormal = 0;
    // Faces are emitted as groups of 4 consecutive vertices.
    for (size_t v = 0; v < mesh.vertices.size(); v += 4) {
      if (Vec3ApproxEquals(mesh.vertices[v].normal, axis))
        facesWithThisNormal++;
    }
    CHECK(facesWithThisNormal == 1);
  }
}

// ===================================================================
// FACE-CULLING TESTS
// ===================================================================

// Two solid blocks sharing a face: the touching faces are hidden and must be
// culled, giving 10 faces instead of 12. This is the first test to exercise
// the "neighbor is solid -> skip" branch — the whole reason the mesher exists.
TEST_CASE("ChunkMesher - adjacent blocks cull their shared face") {
  Chunk chunk;
  chunk.setBlock(0, 0, 0, 1);
  chunk.setBlock(1, 0, 0, 1); // neighbor along +X

  MeshData mesh = ChunkMesher::Build(chunk);

  CHECK(mesh.vertices.size() == 40); // 10 faces * 4 verts (12 - 2 shared)
  CHECK(mesh.indices.size() == 60);  // 10 faces * 6 indices
}

// A fully solid chunk: every interior face is hidden behind a solid neighbor,
// so only the outer shell is emitted (6 sides * SIZE^2 faces). Proves that
// buried cells contribute zero geometry no matter how many there are.
TEST_CASE("ChunkMesher - a solid chunk emits only its outer shell") {
  Chunk chunk;
  for (int z = 0; z < Chunk::SIZE; ++z)
    for (int y = 0; y < Chunk::SIZE; ++y)
      for (int x = 0; x < Chunk::SIZE; ++x)
        chunk.setBlock(x, y, z, 1);

  MeshData mesh = ChunkMesher::Build(chunk);

  const size_t shellFaces = 6 * Chunk::SIZE * Chunk::SIZE; // 6 * 256 = 1536
  CHECK(mesh.vertices.size() == shellFaces * 4);
  CHECK(mesh.indices.size() == shellFaces * 6);
}
