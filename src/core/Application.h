#pragma once

#include "core/LayerStack.h"
#include "core/Window.h"
#include <memory>
class Application {
public:
  Application();
  virtual ~Application() {}
  static Application &Get() { return *s_instance; }
  void Run();
  void PushLayer(std::unique_ptr<Layer> layer);
  void PushOverlay(std::unique_ptr<Layer> layer);
  Window *GetWindow();

private:
  static Application *s_instance;
  Window m_window;
  LayerStack m_stack;
};
