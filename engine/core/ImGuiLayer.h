#pragma once

#include "core/Layer.h"
class ImGuiLayer : public Layer {
public:
  void OnAttach() override;
  void OnDetach() override;
  void Begin();
  void End();
};
