#pragma once

#include "voxel/Chunk.h"
#include "voxel/ChunkMesher.h" // for MeshData

// Greedy mesher: like ChunkMesher, but merges adjacent coplanar faces of the
// same block id into larger rectangles, cutting the quad count dramatically.
// It must cover the *same surface* as ChunkMesher::Build — just with fewer,
// larger quads. (ChunkMesher stays around as the reference oracle in tests.)
class GreedyMesher {
public:
  static MeshData Build(const Chunk &chunk);

private:
  static void
  generateMaskForSlice(const Chunk &chunk,
                       std::array<int, Chunk::SIZE * Chunk::SIZE> &mask,
                       int pos[], int step[], int d, int slice, int u, int v);
  static void emitQuads(MeshData &mesh, int key, int d, int slice, int u, int v,
                        int i, int j, int w, int h);
};
