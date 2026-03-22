#pragma once

#include "rendering/RenderContext.h"
#include "rendering/Shader.h"
#include "scene/Gizmo.h"

class GridGizmo : public Gizmo {
public:
  GridGizmo();
  ~GridGizmo();

  // forbid copy
  GridGizmo(const GridGizmo &) = delete;
  GridGizmo &operator=(const GridGizmo &) = delete;
  // allow move
  GridGizmo(GridGizmo &&) noexcept;
  GridGizmo &operator=(GridGizmo &&) noexcept;
  void On(RenderContext renderContext) override;

private:
  Shader m_shader;
  unsigned int m_VAO, m_VBO;
};
