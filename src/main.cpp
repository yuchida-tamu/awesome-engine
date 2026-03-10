#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <string>

// Core
#include "core/Config.h"
#include "core/Input.h"
#include "core/TextureLoader.h"

#include "cameras/Camera.h"
#include "rendering/PostProcessBlurEffectStrategy.h"
#include "rendering/PostProcessEdgeEffectStrategy.h"
#include "rendering/PostProcessInvertEffectStrategy.h"
#include "rendering/PostProcessing.h"
#include "rendering/RenderContext.h"

#include "meshes/Cube.h"
#include "rendering/Shader.h"
#include "scene/GameObject.h"
#include "scene/RenderComponent.h"
#include "scene/Scene.h"
#include "scene/TransformComponent.h"
#include "scene/WorldSpaceGizmo.h"
#include "stb_image.h"

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

  Input::Initialize(window);

  // tell stb_image.h to flip loaded texture's on the y-axis (before loading
  // model).
  stbi_set_flip_vertically_on_load(true);

  try {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    Camera camera{};

    // Set initial camera position (back away from origin to see the model)
    glm::vec3 cameraPosition = glm::vec3(0.0f, 2.0f, 5.0f);
    camera.UpdatePosition(cameraPosition);

    float cameraSpeed;

    Scene scene{};
    scene.AddCamera(&camera);

    auto cube = std::make_unique<GameObject>();
    auto *transform = cube->AddComponent<TransformComponent>();
    transform->SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));

    Shader cubeShader =
        Shader("shaders/cube.vert.glsl", "shaders/cube.frag.glsl");
    cube->AddComponent<RenderComponent>(std::make_unique<Cube>(), &cubeShader);

    scene.AddGameObject(std ::move(cube));
    //    WorldSpaceGizmo worldSpaceGizmo{};

    // Main loop
    while (!glfwWindowShouldClose(window)) {
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;
      cameraSpeed = Config::DEFAULT_CAMERA_SPEED * deltaTime;

      Input::Update();

      glm::vec2 offset = Input::GetMouseOffset();

      camera.UpdateFront(offset.x, offset.y);
      camera.UpdatePosition(cameraPosition);

      if (Input::IsKeyDown(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
      }

      if (Input::IsKeyHeld(GLFW_KEY_W)) {
        cameraPosition = camera.GetPosition() + camera.GetFront() * cameraSpeed;
      }
      if (Input::IsKeyHeld(GLFW_KEY_S)) {
        cameraPosition = camera.GetPosition() - camera.GetFront() * cameraSpeed;
      }
      if (Input::IsKeyHeld(GLFW_KEY_A)) {
        cameraPosition = camera.GetPosition() - camera.GetRight() * cameraSpeed;
      }
      if (Input::IsKeyHeld(GLFW_KEY_D)) {
        cameraPosition = camera.GetPosition() + camera.GetRight() * cameraSpeed;
      }

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      scene.Update(deltaTime);

      // postprocess.Begin();

      //      worldSpaceGizmo.On(renderContext);
      // postprocess.End();

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
