#pragma once

#include "core/Drawable.h"
#include "rendering/RenderContext.h"
#include "scene/Component.h"
#include <memory>

class RenderComponent : public Component {
public:
  RenderComponent() = default;
  explicit RenderComponent(std::unique_ptr<Drawable> drawable,
                           Shader *shader = nullptr);
  ~RenderComponent() override = default;

  RenderComponent(const RenderComponent &) = delete;
  RenderComponent &operator=(const RenderComponent &) = delete;
  RenderComponent(RenderComponent &&) noexcept = default;
  RenderComponent &operator=(RenderComponent &&) noexcept = default;

  void Draw(RenderContext &renderContext, const float *modelPtr) override;
  bool IsRenderable() const override { return m_drawable != nullptr; }

  void SetDrawable(std::unique_ptr<Drawable> drawable);
  void SetShader(Shader *shader);

private:
  std::unique_ptr<Drawable> m_drawable = nullptr;
  Shader *m_shader = nullptr;
};
