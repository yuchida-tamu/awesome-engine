#pragma once

#include "core/Layer.h"
class EditorLayer : public Layer {
public:
  void OnAttach() override;
  void OnImGuiRender() override;
};
