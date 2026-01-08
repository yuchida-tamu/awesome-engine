#pragma once

#include <cstddef>
#include <memory>

#include "core/Drawable.h"
#include "rendering/RenderContext.h"
#include "rendering/Shader.h"
#include "scene/Transform.h"

class Entity {
public:
  Entity() = default;
  Entity(std::unique_ptr<Drawable> drawable);
  ~Entity() = default;

  Entity(const Entity &) = delete;
  Entity &operator=(const Entity &) = delete;
  Entity(Entity &&) = default;
  Entity &operator=(Entity &&) = default;

  void Draw(Shader &shader, RenderContext &context);
  void Translate(glm::vec3 translate);
  void Scale(glm::vec3 scale);
  void Rotate(float degrees, glm::vec3 direction);

private:
  std::unique_ptr<Drawable> m_drawable = nullptr;
  Transform m_transform{};
};
