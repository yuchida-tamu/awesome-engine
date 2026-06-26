#pragma once
#include "core/Drawable.h"
#include "meshes/Mesh.h"
#include "rendering/Shader.h"
#include "voxel/Chunk.h"
class VoxelChunk : public Drawable {
public:
  explicit VoxelChunk(const Chunk &chunk, int lod);
  void Draw(Shader &shader) override;

  // Quads in this chunk's mesh (6 indices per quad). For debug stats.
  size_t GetQuadCount() const { return m_mesh.GetIndexCount() / 6; }

private:
  int m_lod;
  Mesh m_mesh;
  static Mesh buildMesh(const Chunk &chunk);
};
