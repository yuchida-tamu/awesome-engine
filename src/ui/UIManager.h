#pragma once

#include "core/Config.h"
#include "core/InputListener.h"
#include "glad/glad.h"
#include "ui/UIElement.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <optional>
#include <vector>

#include "rendering/Shader.h"

// Manages all UI elements. Owns an orthographic projection for screen-space
// rendering.
//
// The UIManager is the single entry point for the HUD/overlay system:
//   - Register UI elements (takes ownership via unique_ptr).
//   - Each frame, call Update() then Render() from your main loop.
//   - Render() sets up the orthographic projection, disables depth testing,
//     renders all visible elements, then restores OpenGL state.
//
// Orthographic projection setup (in Render):
//   glm::mat4 projection = glm::ortho(0.0f, screenWidth, 0.0f, screenHeight);
//   This maps pixel coordinates directly: (0,0) = bottom-left, (width,height) =
//   top-right.
//
// Render order: elements are drawn in registration order (first registered =
// drawn first).
//   Later elements draw on top of earlier ones.
class UIManager : public InputListener {
public:
  UIManager(EventBus &eventBus);
  UIManager(EventBus &eventBus, Shader shader);
  ~UIManager() override;

  // Takes ownership of the element. Returns an ID you can use to deregister it.
  int Register(std::unique_ptr<UIElement> element);

  // Removes the element with the given ID. Safe to call with an unknown ID
  // (no-op).
  // TODO: Find and erase the element from m_elements.
  void Deregister(int id);

  // Calls Update(deltaTime) on every registered element.
  void Update(float deltaTime);

  // Renders all visible elements with orthographic projection.
  // TODO: Implement the following steps:
  //   1. Save current OpenGL state (depth test, blend mode).
  //   2. Disable depth testing (glDisable(GL_DEPTH_TEST)).
  //   3. Enable blending (glEnable(GL_BLEND), glBlendFunc(GL_SRC_ALPHA,
  //   GL_ONE_MINUS_SRC_ALPHA)).
  //   4. Set up orthographic projection matrix.
  //   5. Activate the text shader, set the projection uniform.
  //   6. Loop through m_elements: if element->IsVisible(), call
  //   element->Render(shader).
  //   7. Restore the saved OpenGL state.
  void Render();

  int Count() { return m_elements.size(); };

#ifdef UNIT_TEST
public:
#else
private:
#endif
  std::vector<UIElement *> GetVisibleElements();

private:
  struct ElementEntry {
    int id;
    std::unique_ptr<UIElement> element;
  };
  glm::mat4 m_projection;
  std::optional<Shader> m_shader;

  std::vector<ElementEntry> m_elements;
  int m_nextId = 1;
};
