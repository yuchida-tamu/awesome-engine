#include <glad/glad.h>
#include <utility>

#include "rendering/RenderContext.h"
#include "rendering/Shader.h"
#include "scene/GridGizmo.h"

GridGizmo::GridGizmo()
    : m_VAO(0), m_VBO(0),
      m_shader(Shader("shaders/grid_gizmo.vert.glsl",
                      "shaders/grid_gizmo.frag.glsl")) {

  // Large quad on y=0 plane, spanning -100 to +100 on X and Z
  float vertices[] = {
      // Triangle 1
      -100.0f, 0.0f, -100.0f,
       100.0f, 0.0f, -100.0f,
       100.0f, 0.0f,  100.0f,
      // Triangle 2
      -100.0f, 0.0f, -100.0f,
       100.0f, 0.0f,  100.0f,
      -100.0f, 0.0f,  100.0f,
  };

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

GridGizmo::~GridGizmo() {
  if (m_VAO != 0) {
    glDeleteVertexArrays(1, &m_VAO);
  }
  if (m_VBO != 0) {
    glDeleteBuffers(1, &m_VBO);
  }
}

GridGizmo::GridGizmo(GridGizmo &&other) noexcept
    : m_VAO(other.m_VAO), m_VBO(other.m_VBO),
      m_shader(std::move(other.m_shader)) {
  other.m_VAO = 0;
  other.m_VBO = 0;
}

GridGizmo &GridGizmo::operator=(GridGizmo &&other) noexcept {
  if (this != &other) {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);

    m_VAO = other.m_VAO;
    m_VBO = other.m_VBO;
    m_shader = std::move(other.m_shader);

    other.m_VAO = 0;
    other.m_VBO = 0;
  }

  return *this;
}

void GridGizmo::On(RenderContext renderContext) {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);

  m_shader.UseProgram();
  m_shader.SetUniformMatrix4FloatPtr("projection",
                                     renderContext.GetProjectionPtr());
  m_shader.SetUniformMatrix4FloatPtr("view", renderContext.GetViewPtr());

  glBindVertexArray(m_VAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}
