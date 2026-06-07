#include "voxel/ChunkMesher.h"
#include "voxel/Block.h"

namespace {
struct Face {
  int dx, dy, dz; // neighbor offset for the cull check
  glm::vec3 normal;
  glm::vec3 corners[4]; // unit-cube space (0..1), CCW seen from outside
};

constexpr Face kFaces[6] = {
    {1, 0, 0, {1, 0, 0}, {{1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}},   // +X
    {-1, 0, 0, {-1, 0, 0}, {{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}}}, // -X
    {0, 1, 0, {0, 1, 0}, {{0, 1, 1}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}}},   // +Y
    {0, -1, 0, {0, -1, 0}, {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}}, // -Y
    {0, 0, 1, {0, 0, 1}, {{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}},   // +Z
    {0, 0, -1, {0, 0, -1}, {{1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0}}}, // -Z
};

constexpr glm::vec2 kFaceUV[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
} // namespace

MeshData ChunkMesher::Build(const Chunk &chunk) {
  MeshData mesh;
  // Researving memory to prevent memory reallocation
  mesh.vertices.reserve(Chunk::VOLUME);
  mesh.indices.reserve(Chunk::VOLUME);

  // Inner loop is x to match Chunk's x-major layout (cache friendly)
  for (int z = 0; z < Chunk::SIZE; ++z) {
    for (int y = 0; y < Chunk::SIZE; ++y) {
      for (int x = 0; x < Chunk::SIZE; ++x) {
        if (chunk.blockAt(x, y, z) == Chunk::AIR)
          continue;

        glm::vec3 cell(x, y, z);
        for (const Face &face : kFaces) {
          // Check if a face is hidden by a solid neighbor. If so, skip
          if (chunk.blockAt(x + face.dx, y + face.dy, z + face.dz) !=
              Chunk::AIR)
            continue;

          auto base = static_cast<unsigned int>(mesh.vertices.size());
          for (int i = 0; i < 4; ++i) {
            mesh.vertices.push_back({cell + face.corners[i], face.normal,
                                     kFaceUV[i],
                                     blockColor(chunk.blockAt(x, y, z))});
          }

          // Quad = two triangles: (0, 1, 2) and (2, 3, 0)
          for (unsigned int idx : {0u, 1u, 2u, 2u, 3u, 0u}) {
            mesh.indices.push_back(base + idx);
          }
        }
      }
    }
  }

  return mesh;
}
