#pragma once

#include "core/Layer.h"
class EditorLayer : public Layer {
public:
  void OnUpdate(Timestep ts) override;
  void OnImGuiRender() override;
};
