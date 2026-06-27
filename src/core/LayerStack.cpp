#include "core/LayerStack.h"
#include "core/Layer.h"
#include <memory>
#include <utility>

void LayerStack::PushLayer(std::unique_ptr<Layer> layer) {
  layer->OnAttach();
  m_layers.insert(begin() + m_insertIndex, std::move(layer));
  ++m_insertIndex;
}

void LayerStack::PushOverlay(std::unique_ptr<Layer> layer) {
  layer->OnAttach();
  m_layers.push_back(std::move(layer));
}

std::vector<std::unique_ptr<Layer>>::iterator LayerStack::begin() {
  return m_layers.begin();
}

std::vector<std::unique_ptr<Layer>>::iterator LayerStack::end() {
  return m_layers.end();
}

LayerStack::~LayerStack() {
  for (auto &layer : m_layers) {
    layer->OnDetach();
  }
}
