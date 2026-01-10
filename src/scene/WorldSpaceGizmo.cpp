#include <glad/glad.h>
#include <utility>

#include "rendering/RenderContext.h"
#include "rendering/Shader.h"
#include "scene/WorldSpaceGizmo.h"

WorldSpaceGizmo::WorldSpaceGizmo()
    : m_VAO(0), m_VBO(0),
      m_shader(Shader("shaders/gizmo_world_coordinate.vert.glsl",
                      "shaders/gizmo_world_coordinate.geo.glsl",
                      "shaders/gizmo_world_coordinate.frag.glsl")),
      m_model(glm::mat4(1.0f)) {

  float vertices[] = {0.0f, 0.0f, 0.0f};
  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);

  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

WorldSpaceGizmo::~WorldSpaceGizmo() {
  if (m_VAO != 0) {
    glDeleteVertexArrays(1, &m_VAO);
  }
  if (m_VBO != 0) {
    glDeleteBuffers(1, &m_VBO);
  }
}

WorldSpaceGizmo::WorldSpaceGizmo(WorldSpaceGizmo &&other) noexcept
    : m_VAO(other.m_VAO), m_VBO(other.m_VBO),
      m_shader(std::move(other.m_shader)) {
  other.m_VAO = 0;
  other.m_VBO = 0;
}

WorldSpaceGizmo &WorldSpaceGizmo::operator=(WorldSpaceGizmo &&other) noexcept {
  if (this != &other) {
    // Clean up current resources
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);

    m_VAO = other.m_VAO;
    m_VBO = other.m_VBO;
    m_shader = std::move(other.m_shader);

    // Zero out other
    other.m_VAO = 0;
    other.m_VBO = 0;
  }

  return *this;
}

void WorldSpaceGizmo::On(RenderContext renderContext) {
  m_shader.UseProgram();
  m_shader.SetUniformMatrix4FloatPtr("projection",
                                     renderContext.GetProjectionPtr());
  m_shader.SetUniformMatrix4FloatPtr("view", renderContext.GetViewPtr());
  m_shader.SetUniformMatrix4FloatPtr("model", glm::value_ptr(m_model));

  glBindVertexArray(m_VAO);
  glDrawArrays(GL_POINTS, 0, 1);
  glBindVertexArray(0);
}
