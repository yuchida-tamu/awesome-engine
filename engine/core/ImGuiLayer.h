#pragma once

#include "core/Layer.h"
class ImGuiLayer : public Layer {
public:
  ImGuiLayer(bool inputEnabled = false);
  void OnAttach() override;
  void OnDetach() override;
  void Begin();
  void End();

private:
  bool m_isInputEnabled;
};
