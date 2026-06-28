#include "game/GameLayer.h"
#include "core/Application.h"
#include "core/Input.h"
#include "core/TextureLoader.h"
#include "core/Timestep.h"
#include "scene/CameraController.h"
#include "ui/DebugPanel.h"
#include "voxel/TerrainGenerator.h"
#include "world/Coords.h"
#include "world/World.h"
#include <iostream>
#include <memory>

void GameLayer::OnAttach() {
  stbi_set_flip_vertically_on_load(true);

  Input::Initialize(Application::Get().GetWindow(), m_eventBus);

  auto cameraObj = std::make_unique<GameObject>();
  cameraObj->AddComponent<CameraController>(m_camera, m_eventBus);

  m_world.UpdateTerrainConfig(TerrainGenerator::FREQUENCY, 0.00025);
  m_world.UpdateTerrainConfig(TerrainGenerator::NOISE_AMP, 50);

  m_scene.AddCamera(std::move(&m_camera));
  m_scene.AddGameObject(std::move(cameraObj));
}

void GameLayer::OnUpdate(Timestep ts) {
  try {
    Input::Update();

    if (Input::IsKeyDown(GLFW_KEY_ESCAPE)) {
      Application::Get().GetWindow()->Close();
    }

    // Toggle wireframe with F (edge-triggered: one tap = one flip).
    if (Input::IsKeyDown(GLFW_KEY_F)) {
      m_isWireframe = !m_isWireframe;
    }

    // Toggle the debug overlay with F3.
    if (Input::IsKeyDown(GLFW_KEY_F3)) {
      m_debugPanel.Toggle();
    }

    m_fpsCounter.Tick(ts);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Draw the 3D scene in the selected polygon mode.
    glPolygonMode(GL_FRONT_AND_BACK, m_isWireframe ? GL_LINE : GL_FILL);
    m_world.Update(m_scene, m_cubeShader,
                   WorldToChunk(m_camera.GetPosition().x),
                   WorldToChunk(m_camera.GetPosition().z), 2);

    m_scene.Update(ts);

    m_gridGizmo.On(m_scene.GetRenderContext());

    // Reset to fill so the UI (text/buttons) always renders solid.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glm::vec3 camPos = m_camera.GetPosition();
    DebugStats stats{m_fpsCounter.Fps(),     m_world.GetChunkCount(),
                     m_world.GetQuadCount(), camPos,
                     WorldToChunk(camPos.x), WorldToChunk(camPos.z)};
    m_debugPanel.Update(stats);

    m_uiManager.Update(ts);
    m_uiManager.Render();

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    glfwTerminate();
  }
}
