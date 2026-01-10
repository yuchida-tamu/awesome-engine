#pragma once

#include "rendering/RenderContext.h"
#include "rendering/Shader.h"
#include "scene/Gizmo.h"

class WorldSpaceGizmo : public Gizmo {
public:
  WorldSpaceGizmo();
  ~WorldSpaceGizmo();

  // forbid copy
  WorldSpaceGizmo(const WorldSpaceGizmo &) = delete;
  WorldSpaceGizmo &operator=(const WorldSpaceGizmo &) = delete;
  // allow move
  WorldSpaceGizmo(WorldSpaceGizmo &&) noexcept;
  WorldSpaceGizmo &operator=(WorldSpaceGizmo &&) noexcept;
  void On(RenderContext renderContext) override;

private:
  Shader m_shader;
  unsigned int m_VAO, m_VBO;
  glm::mat4 m_model;
};
