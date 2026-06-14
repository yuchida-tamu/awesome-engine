#include "voxel/VoxelChunk.h"
#include "meshes/Mesh.h"
#include "rendering/Shader.h"
#include "voxel/Chunk.h"
#include "voxel/ChunkMesher.h"
#include "voxel/GreedyMesher.h"
#include <utility>

VoxelChunk::VoxelChunk(const Chunk &chunk) : m_mesh(buildMesh(chunk)) {}

void VoxelChunk::Draw(Shader &shader) { m_mesh.Draw(shader); }

Mesh VoxelChunk::buildMesh(const Chunk &chunk) {
  MeshData meshData = GreedyMesher::Build(chunk);
  return Mesh(std::move(meshData.vertices), {}, std::move(meshData.indices));
}
