#include "scene/Entity.h"
#include "rendering/RenderContext.h"

Entity::Entity(std::unique_ptr<Drawable> drawable) {
  m_drawable = std::move(drawable);
}

void Entity::Draw(Shader &shader, RenderContext &context) {
  shader.UseProgram();
  shader.SetUniformMatrix4FloatPtr("projection", context.GetProjectionPtr());
  shader.SetUniformMatrix4FloatPtr("view", context.GetViewPtr());
  shader.SetUniformMatrix4FloatPtr("model", m_transform.GetModelPtr());
  m_drawable->Draw(shader);
}

void Entity::Translate(glm::vec3 translate) { m_transform.Traslate(translate); }

void Entity::Scale(glm::vec3 scale) { m_transform.Scale(scale); }

void Entity::Rotate(float degrees, glm::vec3 direction) {
  m_transform.Roate(degrees, direction);
}
