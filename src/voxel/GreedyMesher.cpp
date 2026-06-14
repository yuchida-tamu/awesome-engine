#include "voxel/GreedyMesher.h"
#include "voxel/Block.h"
#include "voxel/Chunk.h"
#include <array>
#include <cstdint>

// TODO(impl): greedy meshing — 6 passes (±X, ±Y, ±Z). For each pass, sweep
// slices perpendicular to the axis (-1 .. SIZE-1 to catch boundary faces),
// build a 2D mask of visible faces keyed by block id, then greedily merge the
// mask into maximal rectangles and emit one quad per rectangle (winding so the
// normal points outward; vertex color from blockColor(id)).
//
MeshData GreedyMesher::Build(const Chunk &chunk) {
  MeshData mesh;
  std::array<int, Chunk::SIZE * Chunk::SIZE> mask;

  // Loops through each axis (x, y, z)
  for (int d = 0; d < 3; ++d) {
    int u = (d + 1) % 3;
    int v = (d + 2) % 3;
    int pos[3];
    int step[3] = {0, 0, 0};
    step[d] = 1;

    // It needs to check each boundary between cells not a cell itself, which is
    // Chuck::Size count the number of boundary is Chunk::Size + 1, hence
    // `slice` initial value is -1
    for (int slice = -1; slice < Chunk::SIZE; ++slice) {
      // Then, we generate a mask of each slice (u, v)
      generateMaskForSlice(chunk, mask, pos, step, d, slice, u, v);
      // ---- GREEDY MERGE ----
      for (int j = 0; j < Chunk::SIZE; ++j) {
        for (int i = 0; i < Chunk::SIZE;) {
          int key = mask[i + j * Chunk::SIZE];
          if (key == 0) {
            ++i;
            continue;
          }

          // Extend WIDTH along i;
          int w = 1;
          while (i + w < Chunk::SIZE &&
                 mask[(i + w) + j * Chunk::SIZE] == key) {
            ++w;
          }

          // Extend Height along j (whole width-w row must match; same block id)
          int h = 1;
          bool isRowMatched = true;
          while (j + h < Chunk::SIZE && isRowMatched) {
            for (int k = 0; k < w; ++k) {
              if (mask[(i + k) + (j + h) * Chunk::SIZE] != key) {
                isRowMatched = false;
                break;
              }
            }
            if (isRowMatched) {
              ++h;
            }
          }

          // Emit quad
          emitQuads(mesh, key, d, slice, u, v, i, j, w, h);

          // Consume the rectangle so it isn't emitted Generating
          for (int dj = 0; dj < h; ++dj) {
            for (int di = 0; di < w; ++di) {
              mask[(i + di) + (j + dj) * Chunk::SIZE] = 0;
            }
          }
          i += w;
        }
      }
    }
  }

  return mesh;
}

void GreedyMesher::generateMaskForSlice(
    const Chunk &chunk, std::array<int, Chunk::SIZE * Chunk::SIZE> &mask,
    int pos[], int step[], int d, int slice, int u, int v) {
  // Then, we generate a mask of each slice (u, v)
  for (int j = 0; j < Chunk::SIZE; ++j) {
    for (int i = 0; i < Chunk::SIZE; ++i) {
      pos[d] = slice, pos[u] = i, pos[v] = j;
      auto blockId = chunk.BlockAt(pos[0], pos[1], pos[2]);
      auto nextBlockId =
          chunk.BlockAt(pos[0] + step[0], pos[1] + step[1], pos[2] + step[2]);

      int index = i + j * Chunk::SIZE;
      // Storing block id with sign indicating its normal direction
      // (positive: low -> high, negative: low <- high)
      if (IsSolid(blockId) && IsAir(nextBlockId)) {
        mask[index] = (int)blockId;
      } else if (IsAir(blockId) && IsSolid(nextBlockId)) {
        // not blockId since it is Air,
        // we use nextBlockId for the correct block id
        mask[index] = -(int)nextBlockId;
      } else {
        // Both Solid or Both Air
        mask[index] = 0;
      }
    }
  }
}
void GreedyMesher::emitQuads(MeshData &mesh, int key, int d, int slice, int u,
                             int v, int i, int j, int w, int h) {

  int id = std::abs(key);
  glm::vec3 color = BlockColor(id);

  glm::vec3 normal(0.0f);
  normal[d] = (key > 0) ? 1.0f : -1.0f;

  // A corner of the rectangle, build with the pos[] trick (voxel space,
  // 0..SIZE)
  auto corner = [&](int du, int dv) {
    int p[3];
    p[d] = slice + 1;
    p[u] = i + du;
    p[v] = j + dv;
    return glm::vec3(p[0], p[1], p[2]);
  };

  glm::vec3 c0 = corner(0, 0);
  glm::vec3 c1 = corner(w, 0);
  glm::vec3 c2 = corner(w, h);
  glm::vec3 c3 = corner(0, h);

  unsigned int b = (unsigned int)mesh.vertices.size();
  mesh.vertices.push_back({c0, normal, {0, 0}, color});
  mesh.vertices.push_back({c1, normal, {(float)w, 0}, color});
  mesh.vertices.push_back({c2, normal, {(float)w, (float)h}, color});
  mesh.vertices.push_back({c3, normal, {0, (float)h}, color});

  if (key > 0) {
    for (unsigned int k : {0u, 1u, 2u, 0u, 2u, 3u}) {
      mesh.indices.push_back(b + k);
    }
  } else {
    for (unsigned int k : {0u, 2u, 1u, 0u, 3u, 2u}) {
      mesh.indices.push_back(b + k);
    }
  }
}
