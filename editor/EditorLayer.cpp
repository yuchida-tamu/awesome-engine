#include "EditorLayer.h"
#include "core/Timestep.h"
#include "core/Window.h"
#include "imgui.h"

void EditorLayer::OnUpdate(Timestep ts) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void EditorLayer::OnImGuiRender() {
  ImGui::DockSpaceOverViewport();

  ImGui::Begin("Scene Hierachy");
  ImGui::Text("(objects list here)");
  ImGui::End();

  ImGui::Begin("Inspectorr");
  ImGui::Text("(selected object's fields)");
  ImGui::End();

  ImGui::Begin("Viewport");
  ImGui::Text("(rendered world)");
  ImGui::End();
}
