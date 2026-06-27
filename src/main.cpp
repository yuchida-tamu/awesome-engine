#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

// Core
#include "core/EventBus.h"
#include "core/Input.h"
#include "core/InputEvents.h"
#include "core/TextureLoader.h"

#include "cameras/Camera.h"
#include "core/Window.h"
#include "debug/FpsCounter.h"
#include "rendering/Shader.h"
#include "scene/CameraController.h"
#include "scene/GameObject.h"
#include "scene/GridGizmo.h"
#include "scene/Scene.h"
#include "stb_image.h"
#include "ui/ButtonElement.h"
#include "ui/DebugPanel.h"
#include "ui/UIElement.h"
#include "ui/UIManager.h"
#include "voxel/TerrainGenerator.h"
#include "world/Coords.h"
#include "world/World.h"

int main() {
  Window window;
  if (!window.SetUp()) {
    std::cerr << "Failed to setup Window" << std::endl;
  }
  EventBus eventBus;
  Input::Initialize(&window, eventBus);

  // tell stb_image.h to flip loaded texture's on the y-axis (before loading
  // model).
  stbi_set_flip_vertically_on_load(true);

  try {
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    UIManager uiManager(eventBus);

    auto button = std::make_unique<ButtonElement>();
    ButtonElement *buttonRawPtr = button.get();
    button->SetPosition({100.0f, 100.0f});
    button->SetSize({200.0f, 50.0f});
    button->SetColor({0.2f, 0.6f, 1.0f});
    button->SetLabel("Click Me");
    button->SetOnClick([buttonRawPtr](const MouseClickEvent &e) {
      if (e.key == KeyAction::Held && e.button == GLFW_MOUSE_BUTTON_LEFT) {
        buttonRawPtr->SetColor({0.4f, 0.8f, 1.0f});
      } else if (e.key == KeyAction::Up && e.button == GLFW_MOUSE_BUTTON_LEFT) {
        buttonRawPtr->SetColor({0.2f, 0.6f, 1.0f});
      }
    });
    uiManager.Register(std::move(button));

    Camera camera{};

    Scene scene{eventBus};

    auto cameraObj = std::make_unique<GameObject>();
    cameraObj->AddComponent<CameraController>(camera, eventBus);
    scene.AddGameObject(std::move(cameraObj));
    scene.AddCamera(&camera);

    Shader cubeShader("shaders/cube.vert.glsl", "shaders/cube.frag.glsl");

    World world;
    world.UpdateTerrainConfig(TerrainGenerator::FREQUENCY, 0.00025);
    world.UpdateTerrainConfig(TerrainGenerator::NOISE_AMP, 50);

    FpsCounter fpsCounter;
    DebugPanel debugPanel(uiManager);

    GridGizmo gridGizmo{};

    bool wireframe = false; // toggled with F: GL_LINE vs GL_FILL

    glm::vec3 overlayColor{1.0, 0.0, 0.0};

    // Main loop
    while (!window.ShouldClose()) {
      window.BeginFrame();

      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      Input::Update();

      if (Input::IsKeyDown(GLFW_KEY_ESCAPE)) {
        window.Close();
      }

      // Toggle wireframe with F (edge-triggered: one tap = one flip).
      if (Input::IsKeyDown(GLFW_KEY_F)) {
        wireframe = !wireframe;
      }

      // Toggle the debug overlay with F3.
      if (Input::IsKeyDown(GLFW_KEY_F3)) {
        debugPanel.Toggle();
      }

      fpsCounter.Tick(deltaTime);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Draw the 3D scene in the selected polygon mode.
      glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
      world.Update(scene, cubeShader, WorldToChunk(camera.GetPosition().x),
                   WorldToChunk(camera.GetPosition().z), 2);
      scene.Update(deltaTime);
      gridGizmo.On(scene.GetRenderContext());

      // Reset to fill so the UI (text/buttons) always renders solid.
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      // Debug overlay stats (drawn as part of the UI pass below).
      glm::vec3 camPos = camera.GetPosition();
      DebugStats stats{fpsCounter.Fps(),       world.GetChunkCount(),
                       world.GetQuadCount(),   camPos,
                       WorldToChunk(camPos.x), WorldToChunk(camPos.z)};
      debugPanel.Update(stats);

      // UI
      uiManager.Update(deltaTime);
      uiManager.Render();

      window.EndFrame();
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwTerminate();
  return 0;
}
