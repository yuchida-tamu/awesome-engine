#include "rendering/Skybox.h"
#include "rendering/Shader.h"

float skyVertices[] = {
    // positions
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

unsigned int loadCubemap(std::vector<std::string> faces) {
  unsigned int textureID;
  stbi_set_flip_vertically_on_load(false);

  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char *data =
        stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      // Targets for GL_TEXTURE_CUBE_MAP, which is int, is linearly incremented,
      // so we can increment a value of GL_TEXTURE_CUBE_MAP_POSITIVE_X
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cerr << "ERROR: Cubemap texture failed to load at path: " << faces[i]
                << std::endl;
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

Skybox::Skybox() : m_VAO(0), m_VBO(0), m_textuerID(0) {
  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);
  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyVertices), &skyVertices,
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *)0);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Skybox::~Skybox() {
  if (m_VAO != 0) {
    glDeleteVertexArrays(1, &m_VAO);
  }
  if (m_VBO != 0) {
    glDeleteBuffers(1, &m_VBO);
  }
  if (m_textuerID != 0) {
    glDeleteTextures(1, &m_textuerID);
  }
}

Skybox::Skybox(Skybox &&other) noexcept
    : m_VAO(other.m_VAO), m_VBO(other.m_VBO), m_textuerID(other.m_textuerID) {
  other.m_VAO = 0;
  other.m_VBO = 0;
  other.m_textuerID = 0;
}

Skybox &Skybox::operator=(Skybox &&other) noexcept {
  if (this != &other) {
    // Clean up existing resources
    if (m_VAO != 0) {
      glDeleteVertexArrays(1, &m_VAO);
    }
    if (m_VBO != 0) {
      glDeleteBuffers(1, &m_VBO);
    }
    if (m_textuerID != 0) {
      glDeleteTextures(1, &m_textuerID);
    }

    // Transfer ownership
    m_VAO = other.m_VAO;
    m_VBO = other.m_VBO;
    m_textuerID = other.m_textuerID;

    // Reset source
    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_textuerID = 0;
  }
  return *this;
}

void Skybox::SetTextures(std::vector<std::string> paths) {
  m_textuerID = loadCubemap(paths);
}

void Skybox::Draw(Shader &shader) {
  glDepthFunc(GL_LEQUAL);
  glDepthMask(GL_FALSE);
  glBindVertexArray(m_VAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_textuerID);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);
}
