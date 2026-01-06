#pragma once

#include "rendering/Shader.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <string>
#include <vector>

class Drawable {
public:
  virtual void Draw(Shader &shader) = 0;
};

class Skybox : public Drawable {
public:
  Skybox();
  ~Skybox();
  void SetTextures(std::vector<std::string> paths);
  void Draw(Shader &shader) override;

private:
  unsigned int m_VAO, m_VBO;
  unsigned int m_textuerID;
};
