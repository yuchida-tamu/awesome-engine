#pragma once

#include <glm/glm.hpp>

class Shader;

// Abstract base class for all UI primitives (text labels, panels, buttons,
// etc.).
//
// UI elements live in screen-space pixel coordinates:
//   - m_position is the bottom-left corner of the element in pixels
//     (0,0 = bottom-left of the window).
//   - m_size is the width and height in pixels.
//
// Subclasses must implement:
//   - Render(Shader&): draw the element using the provided shader.
//   - Update(float deltaTime): tick logic (animations, blinking cursor, etc.).
class UIElement {
public:
  virtual ~UIElement() = default;

  // --- Pure virtuals — subclasses fill these in ---
  virtual void Render(Shader &shader) = 0;
  virtual void Update(float deltaTime) = 0;

  // --- Position / size ---
  void SetPosition(glm::vec2 position) { m_position = position; }
  glm::vec2 GetPosition() const { return m_position; }

  void SetSize(glm::vec2 size) { m_size = size; }
  glm::vec2 GetSize() const { return m_size; }

  // --- Visibility ---
  void SetVisible(bool visible) { m_visible = visible; }
  bool IsVisible() const { return m_visible; }

protected:
  glm::vec2 m_position{0.0f, 0.0f};
  glm::vec2 m_size{0.0f, 0.0f};
  bool m_visible = true;
};
