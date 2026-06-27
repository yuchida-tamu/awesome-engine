#pragma once

#include "core/Layer.h"

#include <memory>
#include <vector>
class LayerStack {
public:
  ~LayerStack();
  void PushLayer(std::unique_ptr<Layer> layer);
  void PushOverlay(std::unique_ptr<Layer> layer);
  std::vector<std::unique_ptr<Layer>>::iterator begin();
  std::vector<std::unique_ptr<Layer>>::iterator end();

private:
  std::vector<std::unique_ptr<Layer>> m_layers;
  unsigned m_insertIndex = 0;
};
