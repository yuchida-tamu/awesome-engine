#include "EditorLayer.h"
#include "core/Timestep.h"
#include "core/Window.h"
#include "imgui.h"
#include "imgui_internal.h"

void EditorLayer::OnUpdate(Timestep ts) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void EditorLayer::OnImGuiRender() {
  ImGuiID dockspaceId = ImGui::GetID("Awesome Engine Editor");
  ImGuiViewport *viewport = ImGui::GetMainViewport();

  // Setup editor layout on the first frame
  if (!m_isInitialized) {
    m_isInitialized = true;

    if (ImGui::DockBuilderGetNode(dockspaceId) == nullptr) {
      ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
      ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->Size);
      ImGuiID dockIdLeft = 0;
      ImGuiID dockIdMain = dockspaceId;
      ImGui::DockBuilderSplitNode(dockIdMain, ImGuiDir_Left, 0.20f, &dockIdLeft,
                                  &dockIdMain);
      ImGuiID dockIdRight = 0;
      ImGui::DockBuilderSplitNode(dockIdMain, ImGuiDir_Left, 0.80f, &dockIdMain,
                                  &dockIdRight);
      ImGui::DockBuilderDockWindow("Viewport", dockIdMain);
      ImGui::DockBuilderDockWindow("Scene Hierachy", dockIdLeft);
      ImGui::DockBuilderDockWindow("Inspector", dockIdRight);

      ImGui::DockBuilderFinish(dockspaceId);
    }
  }

  ImGui::DockSpaceOverViewport(dockspaceId, viewport,
                               ImGuiDockNodeFlags_PassthruCentralNode);

  ImGui::Begin("Scene Hierachy");
  ImGui::Text("(objects list here)");
  ImGui::End();

  ImGui::Begin("Inspector");
  ImGui::Text("(selected object's fields)");
  ImGui::End();

  ImGui::Begin("Viewport");
  ImGui::Text("(rendered world)");
  ImGui::End();
}
