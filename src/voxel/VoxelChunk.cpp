#include "voxel/VoxelChunk.h"
#include "meshes/Mesh.h"
#include "rendering/Shader.h"
#include "voxel/Block.h"
#include "voxel/Chunk.h"
#include "voxel/ChunkMesher.h"
#include "voxel/GreedyMesher.h"
#include <glm/gtc/type_ptr.hpp>
#include <utility>

VoxelChunk::VoxelChunk(const Chunk &chunk, int lod)
    : m_mesh(buildMesh(chunk)), m_lod(lod) {}

void VoxelChunk::Draw(Shader &shader) {
  shader.SetUniformVec3("colorOverlay", glm::value_ptr(GetColorByLOD(m_lod)));
  m_mesh.Draw(shader);
}

Mesh VoxelChunk::buildMesh(const Chunk &chunk) {
  // MeshData meshData = GreedyMesher::Build(chunk);
  MeshData meshData = ChunkMesher::Build(chunk);
  return Mesh(std::move(meshData.vertices), {}, std::move(meshData.indices));
}
