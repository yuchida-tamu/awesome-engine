#pragma once

#include "core/InputEvents.h"
#include "rendering/Shader.h"
#include "ui/Clickable.h"
#include "ui/UIElement.h"
#include <functional>
#include <optional>

class ButtonElement : public UIElement, public Clickable {
public:
  ButtonElement();
  ~ButtonElement() override;

  using ClickCallBack = std::function<void(const MouseClickEvent &e)>;
  // Register Callback, which is triggered when the UI is pressed
  void SetOnClick(ClickCallBack callback);
  void OnClick(const MouseClickEvent &e) override;

  // --- Appearance ---
  void SetColor(glm::vec3 color);
  glm::vec3 GetColor() const;

  void Render(const glm::mat4 &projection) override;
  void Update(float deltaTime) override;

private:
  ClickCallBack m_onClick;
  glm::vec3 m_color{1.0f, 1.0f, 1.0f};
  std::optional<Shader> m_shader;
  unsigned int m_vao = 0;
  unsigned int m_vbo = 0;
};
