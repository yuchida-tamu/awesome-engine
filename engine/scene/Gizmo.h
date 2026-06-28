#pragma once

#include "rendering/RenderContext.h"
class Gizmo {
  virtual void On(RenderContext renderContext) = 0;
};
