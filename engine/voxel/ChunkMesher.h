#pragma once

#include <vector>

#include "meshes/Mesh.h"
#include "voxel/Chunk.h"

// The CPU-side result of meshing a chunk: raw geometry buffers, no OpenGL.
// Hand these to a Mesh to upload them to the GPU.
struct MeshData {
  std::vector<Mesh::Vertex> vertices;
  std::vector<unsigned int> indices;
};

// Turns voxel data into a renderable surface using face culling: a quad is
// emitted only where a solid cell borders empty space. Pure and OpenGL-free
// (same chunk in -> same geometry out), which is what makes it unit-testable.
class ChunkMesher {
public:
  static MeshData Build(const Chunk &chunk);
};
