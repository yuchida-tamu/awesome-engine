#include "scene/RenderComponent.h"
#include <memory>

RenderComponent::RenderComponent(std::unique_ptr<Drawable> drawable,
                                 Shader *shader)
    : m_drawable(std::move(drawable)), m_shader(shader) {}

void RenderComponent::SetDrawable(std::unique_ptr<Drawable> drawable) {
  m_drawable = std::move(drawable);
}

void RenderComponent::SetShader(Shader *shader) { m_shader = shader; }

void RenderComponent::Draw(RenderContext &context, const float *modelPtr) {
  m_shader->UseProgram();
  // FIXME: "projection", "view", "model" could be source of confusion and
  // potential bugs, Perhaps, I should convert them to enums so that they
  // provide more clarity and consistency.
  m_shader->SetUniformMatrix4FloatPtr("projection", context.GetProjectionPtr());
  m_shader->SetUniformMatrix4FloatPtr("view", context.GetViewPtr());
  m_shader->SetUniformMatrix4FloatPtr("model", modelPtr);

  m_drawable->Draw(*m_shader);
}
