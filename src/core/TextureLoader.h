#pragma once

#include "stb_image.h"
#include <glad/glad.h>
#include <vector>

class TextureLoader {
public:
  TextureLoader(std::vector<std::string> filepaths);
  ~TextureLoader();

  // Rule of 5: Delete copy operations (OpenGL handles can't be safely copied)
  TextureLoader(const TextureLoader &) = delete;
  TextureLoader &operator=(const TextureLoader &) = delete;

  // Allow move operations for efficiency
  TextureLoader(TextureLoader &&other) noexcept;
  TextureLoader &operator=(TextureLoader &&other) noexcept;

  void Bind();

private:
  std::vector<GLuint> m_textures;
  void Load2D(GLuint *texture, std::string &filepath);
  void Clear();
};
