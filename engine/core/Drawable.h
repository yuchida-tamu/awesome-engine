#pragma once

#include "rendering/Shader.h"

/**
 * @brief Interface for objects that can be rendered.
 *
 * Inherit from this class to make an object drawable by the rendering system.
 */
class Drawable {
public:
  virtual ~Drawable() = default;
  virtual void Draw(Shader &shader) = 0;
};
