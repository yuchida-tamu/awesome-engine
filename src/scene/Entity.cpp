#include "scene/Entity.h"
#include "rendering/RenderContext.h"
#include "rendering/Skybox.h"

Entity::Entity(std::unique_ptr<Drawable> drawable) {
  m_drawable = std::move(drawable);
}

void Entity::Draw(Shader &shader, RenderContext &context) {
  shader.UseProgram();
  shader.SetUniformMatrix4FloatPtr("projection", context.GetProjectionPtr());
  shader.SetUniformMatrix4FloatPtr("view", context.GetViewPtr());
  m_drawable->Draw(shader);
}
