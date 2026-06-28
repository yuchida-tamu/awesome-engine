#include "core/Application.h"
#include "core/ImGuiLayer.h"
#include "core/LayerStack.h"
#include "core/Timestep.h"
#include "core/Window.h"
#include <iostream>
#include <memory>

Application *Application::s_instance = nullptr;

Application::Application() {
  s_instance = this;

  if (!m_window.SetUp()) {
    std::cerr << "Failed to set up Window" << std::endl;
    return;
  }

  auto imgui = std::make_unique<ImGuiLayer>();
  m_imguiLayer = imgui.get();

  PushOverlay(std::move(imgui));
}

void Application::Run() {
  float deltaTs;
  float lastTs = m_window.GetTime();

  while (!m_window.ShouldClose()) {
    m_window.BeginFrame();
    float currentTime = m_window.GetTime();
    deltaTs = currentTime - lastTs;
    lastTs = currentTime;

    m_imguiLayer->Begin();
    for (auto &layer : m_stack) {
      layer->OnUpdate(deltaTs);
      layer->OnImGuiRender();
    }
    m_imguiLayer->End();

    m_window.EndFrame();
  }
}

void Application::PushLayer(std::unique_ptr<Layer> layer) {
  m_stack.PushLayer(std::move(layer));
}

void Application::PushOverlay(std::unique_ptr<Layer> layer) {
  m_stack.PushOverlay(std::move(layer));
}

Window *Application::GetWindow() { return &m_window; }
