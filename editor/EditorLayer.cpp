#include "EditorLayer.h"
#include "core/Application.h"
#include "core/Input.h"
#include "core/TextureLoader.h"
#include "core/Timestep.h"
#include "core/Window.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "scene/CameraController.h"
#include "world/Coords.h"
#include <cstdint>

void EditorLayer::OnAttach() {
  stbi_set_flip_vertically_on_load(true);

  Input::Initialize(Application::Get().GetWindow(), m_eventBus);

  auto cameraObj = std::make_unique<GameObject>();
  cameraObj->AddComponent<CameraController>(m_camera, m_eventBus);

  m_world.UpdateTerrainConfig(TerrainGenerator::FREQUENCY, 0.00025);
  m_world.UpdateTerrainConfig(TerrainGenerator::NOISE_AMP, 50);

  m_scene.AddCamera(&m_camera);
  m_scene.AddGameObject(std::move(cameraObj));
}

void EditorLayer::OnUpdate(Timestep ts) {
  m_fb.Bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_CULL_FACE);

  // render scenes
  try {
    Input::Update();

    // Draw the 3D scene in the selected polygon mode.
    glPolygonMode(GL_FRONT_AND_BACK, m_isWireframe ? GL_LINE : GL_FILL);
    m_world.Update(m_scene, m_cubeShader,
                   WorldToChunk(m_camera.GetPosition().x),
                   WorldToChunk(m_camera.GetPosition().z), 2);

    m_scene.Update(ts);

    m_uiManager.Update(ts);
    m_uiManager.Render();

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    glfwTerminate();
  }
  m_fb.Unbind();
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
  auto panelSize = ImGui::GetContentRegionAvail();
  ImGui::Image((ImTextureID)(intptr_t)m_fb.GetColorAttachment(), panelSize,
               {0, 1}, {1, 0});

  if (panelSize.x > 0 && panelSize.y > 0 &&
      (panelSize.x != m_fb.GetWidth() || panelSize.y != m_fb.GetHeight())) {
    m_fb.Resize(panelSize.x, panelSize.y);
    m_scene.SetViewPortSize(panelSize.x, panelSize.y);
  }

  ImGui::End();
}
