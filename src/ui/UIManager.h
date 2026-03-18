#pragma once

#include "core/Config.h"
#include "core/EventBus.h"
#include "core/InputEvents.h"
#include "core/InputListener.h"
#include "ui/Clickable.h"
#include "ui/UIElement.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

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
  ~UIManager() override;

  // Takes ownership of the element. Returns an ID you can use to deregister it.
  int Register(std::unique_ptr<UIElement> element);

  // Removes the element with the given ID. Safe to call with an unknown ID
  // (no-op).
  void Deregister(int id);

  // Calls Update(deltaTime) on every registered element.
  void Update(float deltaTime);

  // Renders all visible elements with orthographic projection.
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

  SubscriptionID m_clickSub;
  glm::mat4 m_projection;

  std::vector<ElementEntry> m_elements;
  int m_nextId = 1;

  void OnMouseClick(const MouseClickEvent &e);
};
