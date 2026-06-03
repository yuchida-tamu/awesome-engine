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
