#include <algorithm>
#include <glad/glad.h>

#include "FastNoiseLite.h"
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
#include "core/Config.h"
#include "core/EventBus.h"
#include "core/Input.h"
#include "core/InputEvents.h"
#include "core/TextureLoader.h"

#include "cameras/Camera.h"
#include "meshes/Model.h"
#include "rendering/PostProcessBlurEffectStrategy.h"
#include "rendering/PostProcessEdgeEffectStrategy.h"
#include "rendering/PostProcessInvertEffectStrategy.h"
#include "rendering/PostProcessing.h"
#include "rendering/RenderContext.h"

#include "meshes/Cube.h"
#include "rendering/Shader.h"
#include "scene/CameraController.h"
#include "scene/GameObject.h"
#include "scene/GridGizmo.h"
#include "scene/RenderComponent.h"
#include "scene/Scene.h"
#include "scene/TransformComponent.h"
#include "scene/WorldSpaceGizmo.h"
#include "stb_image.h"
#include "ui/ButtonElement.h"
#include "ui/TextElement.h"
#include "ui/UIElement.h"
#include "ui/UIManager.h"
#include "voxel/Chunk.h"
#include "voxel/VoxelChunk.h"

void error_callback(int error, const char *description) {
  std::cerr << "GLFW Error: " << description << std::endl;
}

int main() {
  // Set the error callback before initializing GLFW
  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  // Set window hints for OpenGL 4.1 Core Profile
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window =
      glfwCreateWindow(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT,
                       "Awesome Engine", NULL, NULL);

  if (window == NULL) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLAD after create the context
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  EventBus eventBus;
  Input::Initialize(window, eventBus);

  // tell stb_image.h to flip loaded texture's on the y-axis (before loading
  // model).
  stbi_set_flip_vertically_on_load(true);

  try {
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    UIManager uiManager(eventBus);
    auto text = std::make_unique<TextElement>();
    text->SetText("Awesome Engine");
    text->SetPosition(glm::vec2(12.0, 12.0));
    text->SetColor(glm::vec3(1.0, 0.1, 0.4));
    // uiManager.Register(std::move(text));

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

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetSeed(1337);
    noise.SetFrequency(0.05f);

    int worldSize = 8; // grid is worldSize x worldSize chunks
    for (int chunkX = 0; chunkX < worldSize; ++chunkX) {
      for (int chunkZ = 0; chunkZ < worldSize; ++chunkZ) {
        Chunk chunk; // fresh chunk per grid cell

        for (int z = 0; z < Chunk::SIZE; ++z) {
          for (int x = 0; x < Chunk::SIZE; ++x) {
            float n = noise.GetNoise((float)(chunkX * Chunk::SIZE + x),
                                     (float)(chunkZ * Chunk::SIZE + z));
            int height =
                (int)((n + 1.0f) * 0.5f * 14) + 1; // remap [-1, 1] -> [1, 15]
            for (int y = 0; y < height; ++y) {
              chunk.setBlock(x, y, z, 1);
            }
          }
        }

        // Build the renderable once, after the chunk is fully filled.
        auto chunkObj = std::make_unique<GameObject>();
        chunkObj->AddComponent<TransformComponent>()->SetPosition(
            {chunkX * Chunk::SIZE, 0, chunkZ * Chunk::SIZE});
        chunkObj->AddComponent<RenderComponent>(
            std::make_unique<VoxelChunk>(chunk), &cubeShader);
        scene.AddGameObject(std::move(chunkObj));
      }
    }

    //    WorldSpaceGizmo worldSpaceGizmo{};
    GridGizmo gridGizmo{};

    // Main loop
    while (!glfwWindowShouldClose(window)) {
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      Input::Update();

      if (Input::IsKeyDown(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
      }

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      scene.Update(deltaTime);
      gridGizmo.On(scene.GetRenderContext());

      // UI
      uiManager.Update(deltaTime);
      uiManager.Render();

      // Swap buffers and poll events
      glfwSwapBuffers(window);
      glfwPollEvents();
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwTerminate();
  return 0;
}
