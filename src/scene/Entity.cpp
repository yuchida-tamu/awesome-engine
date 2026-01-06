#include "scene/Entity.h"
#include "rendering/RenderContext.h"

Entity::Entity(std::unique_ptr<Drawable> drawable) {
  m_drawable = std::move(drawable);
}

Entity::~Entity() { m_drawable = nullptr; }

void Entity::Draw(Shader &shader, RenderContext &context) {
  shader.UseProgram();
  shader.SetUniformMatrix4FloatPtr("projection", context.GetProjectionPtr());
  shader.SetUniformMatrix4FloatPtr("view", context.GetViewPtr());
  m_drawable->Draw(shader);
}
