#pragma once

#include "core/InputEvents.h"
class Clickable {
public:
  virtual ~Clickable() = default;
  virtual void OnClick(const MouseClickEvent &) = 0;
};
