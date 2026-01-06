#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
 * @brief Holds per-frame rendering matrices form the camera
 *
 * Shared across all objects in a scene. Set once per frame before rendering a
 * scene.
 */
class RenderContext {
public:
  const float *GetProjectionPtr() { return glm::value_ptr(m_projection); }
  const float *GetViewPtr() { return glm::value_ptr(m_view); }
  void SetProjection(glm::mat4 projection) { m_projection = projection; }
  void SetView(glm::mat4 view) { m_view = view; }

private:
  glm::mat4 m_projection, m_view;
};
