#pragma once

#include "core/EventBus.h"
#include "core/Layer.h"
#include "debug/FpsCounter.h"
#include "rendering/Shader.h"
#include "scene/GridGizmo.h"
#include "scene/Scene.h"
#include "ui/DebugPanel.h"
#include "ui/UIManager.h"
#include "world/World.h"

class GameLayer : public Layer {
public:
  GameLayer()
      : m_scene(m_eventBus), m_uiManager(m_eventBus), m_debugPanel(m_uiManager),
        m_cubeShader("shaders/cube.vert.glsl", "shaders/cube.frag.glsl") {}
  void OnAttach() override;
  void OnUpdate(Timestep ts) override;
  void OnImGuiRender() override;

private:
  EventBus m_eventBus;
  Scene m_scene;
  World m_world;
  Camera m_camera;
  UIManager m_uiManager;

  Shader m_cubeShader;

  FpsCounter m_fpsCounter;
  DebugPanel m_debugPanel;
  GridGizmo m_gridGizmo;
  bool m_isWireframe = false;
};
