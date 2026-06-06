#pragma once
#include "core/Drawable.h"
#include "meshes/Mesh.h"
#include "rendering/Shader.h"
#include "voxel/Chunk.h"
class VoxelChunk : public Drawable {
public:
  explicit VoxelChunk(const Chunk &chunk);
  void Draw(Shader &shader) override;

private:
  Mesh m_mesh;
  static Mesh buildMesh(const Chunk &chunk);
};
