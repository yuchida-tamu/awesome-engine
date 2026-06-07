#include "Mesh.h"
#include <cstddef> // For offsetof
#include <iostream>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<Texture> textures,
           std::vector<unsigned int> indices, glm::vec4 baseColorFactor) {
  m_vertices = vertices;
  m_textures = textures;
  m_indices = indices;
  m_baseColorFactor = baseColorFactor;

  initialize();
}
Mesh::~Mesh() { cleanup(); }

// Move constructor
Mesh::Mesh(Mesh &&other) noexcept
    : m_VAO(other.m_VAO), m_VBO(other.m_VBO), m_EBO(other.m_EBO),
      m_vertices(std::move(other.m_vertices)),
      m_textures(std::move(other.m_textures)),
      m_indices(std::move(other.m_indices)),
      m_baseColorFactor(other.m_baseColorFactor) {
  // Reset the moved-from object's handles so it doesn't delete our buffers
  other.m_VAO = 0;
  other.m_VBO = 0;
  other.m_EBO = 0;
}

// Move assignment operator
Mesh &Mesh::operator=(Mesh &&other) noexcept {
  if (this != &other) {
    // Clean up current resources
    cleanup();

    // Move resources from other
    m_VAO = other.m_VAO;
    m_VBO = other.m_VBO;
    m_EBO = other.m_EBO;
    m_vertices = std::move(other.m_vertices);
    m_textures = std::move(other.m_textures);
    m_indices = std::move(other.m_indices);
    m_baseColorFactor = other.m_baseColorFactor;

    // Reset the moved-from object's handles
    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;
  }
  return *this;
}

void Mesh::Draw(Shader &shader) {
  // Safety check: don't draw if VAO is not initialized
  if (m_VAO == 0) {
    std::cerr << "[DEBUG] Error VAO is not initialized" << std::endl;
    return;
  }

  // Safety check: don't draw if there are no indices
  if (m_indices.empty()) {
    std::cerr << "Warning: Mesh has no indices to draw" << std::endl;
    return;
  }

  unsigned int diffuseNr = 1;
  unsigned int specularNr = 1;
  bool hasDiffuseTexture = false;

  for (unsigned int i = 0; i < m_textures.size(); i++) {
    // Skip invalid texture IDs (0 means texture failed to load)
    if (m_textures[i].id == 0) {
      continue;
    }

    // retrieve texture number (the N in diffuse_textureN)
    std::string number;
    std::string name = m_textures[i].type;
    std::string uniformName;

    if (name == "texture_diffuse") {
      number = std::to_string(diffuseNr++);
      uniformName = name + number; // "texture_diffuse1"
      hasDiffuseTexture = true;
    } else if (name == "texture_specular") {
      number = std::to_string(specularNr++);
      uniformName = name + number; // "texture_specular1"
    } else {
      std::cerr << "[DEBUG] Error Unknown texture type is specified: " << name
                << std::endl;
      continue; // Skip unknown texture types
    }

    // Activate texture unit and bind texture
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, m_textures[i].id);

    // Set uniform - shader expects "texture_diffuse1", not
    // "material.texture_diffuse1"
    shader.SetUniformInt(uniformName.c_str(), i);
  }

  // If no diffuse texture was found, bind a default white texture
  if (!hasDiffuseTexture) {
    static unsigned int defaultTexture = 0;
    if (defaultTexture == 0) {
      glGenTextures(1, &defaultTexture);
      glBindTexture(GL_TEXTURE_2D, defaultTexture);
      unsigned char whitePixel[] = {255, 255, 255, 255};
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, whitePixel);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, defaultTexture);
    shader.SetUniformInt("texture_diffuse1", 0);
  } else {
    glActiveTexture(GL_TEXTURE0);
  }

  // Set base color factor (tints texture or provides solid color when no
  // texture)
  shader.SetUniformVec4("baseColorFactor", &m_baseColorFactor[0]);

  // draw mesh
  // Binding VAO automatically binds the EBO that was bound when VAO was created
  glBindVertexArray(m_VAO);
  glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void Mesh::initialize() {
  // Safety check: don't initialize if there are no vertices or indices
  if (m_vertices.empty() || m_indices.empty()) {
    std::cerr << "Warning: Attempting to initialize mesh with empty vertex or "
                 "index data."
              << std::endl;
    return;
  }

  // Generate and bind VAO first
  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);
  glGenBuffers(1, &m_EBO);

  // Bind VAO - all subsequent buffer operations will be associated with this
  // VAO
  glBindVertexArray(m_VAO);

  // Bind and upload VBO data (vertex data)
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
               &m_vertices[0], GL_STATIC_DRAW);

  // Bind and upload EBO data (index data)
  // IMPORTANT: EBO must be bound while VAO is active to become part of VAO
  // state
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
               &m_indices[0], GL_STATIC_DRAW);

  // Set up vertex attributes (these are stored in the VAO)
  // positions attribute (layout = 0)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

  // normals attribute (layout = 1)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, normal));

  // texCoords attribute (layout = 2)
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, texCoords));

  // color attributes (layout = 3)
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, color));

  // Unbind VAO (EBO binding is preserved in VAO state)
  glBindVertexArray(0);

  // Note: VBO and EBO remain bound globally, but this is fine since they're not
  // used directly The VAO stores references to them, so we can unbind them if
  // desired (optional)
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::cleanup() {
  if (m_VAO != 0) {
    glDeleteVertexArrays(1, &m_VAO);
    m_VAO = 0;
  }
  if (m_VBO != 0) {
    glDeleteBuffers(1, &m_VBO);
    m_VBO = 0;
  }
  if (m_EBO != 0) {
    glDeleteBuffers(1, &m_EBO);
    m_EBO = 0;
  }
}
