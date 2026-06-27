#pragma once

#include "core/LayerStack.h"
#include "core/Window.h"
#include <memory>
class Application {
public:
  Application();
  virtual ~Application() {}
  void Run();
  void PushLayer(std::unique_ptr<Layer> layer);
  void PushOverlay(std::unique_ptr<Layer> layer);

private:
  Window m_window;
  LayerStack m_stack;
};
