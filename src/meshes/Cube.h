#pragma once

#include <vector>

#include "Mesh.h"
#include "core/TextureLoader.h"
#include "core/Drawable.h"
#include "rendering/Shader.h"

class Cube : public Drawable {
public:
  Cube();
  Cube(std::string path);
  ~Cube();
  void Draw(Shader &shader) override;
    void SetTexture(std::string path);

private:
    Mesh m_mesh;
    Mesh createMesh();
    Mesh createMeshFromTexture(std::string path);
};