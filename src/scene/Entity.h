#pragma once

#include <cstddef>
#include <memory>

#include "core/Drawable.h"
#include "rendering/RenderContext.h"
#include "rendering/Shader.h"

class Entity {
public:
  Entity() = default;
  Entity(std::unique_ptr<Drawable> drawable);
  ~Entity();
  void Draw(Shader &shader, RenderContext &context);

private:
  std::unique_ptr<Drawable> m_drawable = nullptr;
};
