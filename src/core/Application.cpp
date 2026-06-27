#include "core/Application.h"
#include "core/LayerStack.h"
#include "core/Timestep.h"
#include "core/Window.h"
#include <iostream>
#include <memory>

Application::Application() {
  if (!m_window.SetUp()) {
    std::cerr << "Failed to set up Window" << std::endl;
    return;
  }
}

void Application::Run() {
  float deltaTs;
  float lastTs = m_window.GetTime();

  while (!m_window.ShouldClose()) {
    m_window.BeginFrame();
    // deltaTime
    float currentTime = m_window.GetTime();
    deltaTs = currentTime - lastTs;
    lastTs = currentTime;

    for (auto &layer : m_stack) {
      layer->OnUpdate(deltaTs);
    }

    m_window.EndFrame();
  }
}

void Application::PushLayer(std::unique_ptr<Layer> layer) {
  m_stack.PushLayer(std::move(layer));
}

void Application::PushOverlay(std::unique_ptr<Layer> layer) {
  m_stack.PushOverlay(std::move(layer));
}
