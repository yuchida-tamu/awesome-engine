#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>

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
#include "rendering/Shader.h"
#include "rendering/Skybox.h"

#include "meshes/Cube.h"
#include "meshes/Model.h"
#include "scene/Entity.h"
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

  std::vector<std::string> grassTexturePaths{"textures/grass.png"};
  TextureLoader grassTexture{grassTexturePaths};

  // tell stb_image.h to flip loaded texture's on the y-axis (before loading
  // model).
  stbi_set_flip_vertically_on_load(true);

  try {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    Camera camera{};

    // Set initial camera position (back away from origin to see the model)
    glm::vec3 cameraPosition = glm::vec3(0.0f, 2.0f, 5.0f);
    camera.UpdatePosition(cameraPosition);

    float cameraSpeed;

    RenderContext renderContext;

    Shader shader{"shaders/simple_model.vert", "shaders/fog.frag"};
    Entity floor(std::move(std::make_unique<Model>("models/floor/floor.glb")));
    floor.Translate(glm::vec3(0.0f, 0.0f, 0.0f));
    floor.Rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    floor.Scale(glm::vec3(1.0f, 1.0f, 1.0f));

    Entity cube(std::move(std::make_unique<Cube>("textures/container.png")));
    cube.Translate(glm::vec3(2.0f, 0.5f, -5.0f));

    Shader backpackShader{"shaders/simple_model.vert",
                          "shaders/simple_model.frag"};
    Entity backpack(
        std::move(std::make_unique<Model>("models/backpack/backpack.obj")));
    backpack.Translate(glm::vec3(4.0f, 1.0f, -4.0f));
    backpack.Scale(glm::vec3(0.5f, 0.5f, 0.5f));

    Shader gizmoShader("shaders/gizmo_normal.vert", "shaders/gizmo_normal.geo",
                       "shaders/gizmo_normal.frag");
    Shader gizmoWorldCoordinateShader(
        "shaders/gizmo_world_coordinate.vert.glsl",
        "shaders/gizmo_world_coordinate.geo.glsl",
        "shaders/gizmo_world_coordinate.frag.glsl");

    Shader skyboxShader{"shaders/skybox.vert", "shaders/skybox.frag"};
    std::vector<std::string> faces{
        "textures/skybox/right.jpg", "textures/skybox/left.jpg",
        "textures/skybox/top.jpg",   "textures/skybox/bottom.jpg",
        "textures/skybox/front.jpg", "textures/skybox/back.jpg",
    };
    auto skybox = std::make_unique<Skybox>();
    skybox->SetTextures(faces);
    Entity skyboxEntity(std::move(skybox));
    // TODO:Create a world center gizmo  class
    unsigned int gizmoVAO, gizmoVBO;
    float gizmoVertex[] = {0.0, 0.0, 0.0};
    glGenVertexArrays(1, &gizmoVAO);
    glGenBuffers(1, &gizmoVBO);
    glBindVertexArray(gizmoVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gizmoVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gizmoVertex), gizmoVertex,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Calculate projection matrix once (doesn't change unless window is
    // resized)
    glm::mat4 projection =
        glm::perspective(glm::radians(Config::FOV),
                         static_cast<float>(Config::WINDOW_WIDTH) /
                             static_cast<float>(Config::WINDOW_HEIGHT),
                         Config::NEAR_PLANE, Config::FAR_PLANE);

    PostProcessing postprocess{};
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

      if (Input::IsKeyHeld(GLFW_KEY_0)) {
        postprocess.ClearStrategy();
      }

      if (Input::IsKeyHeld(GLFW_KEY_1)) {
        postprocess.SetStrategy(
            std::make_unique<PostProcessBlurEffectStrategy>());
      }

      if (Input::IsKeyHeld(GLFW_KEY_2)) {
        postprocess.SetStrategy(
            std::make_unique<PostProcessInvertEffectStrategy>());
      }

      if (Input::IsKeyHeld(GLFW_KEY_3)) {
        postprocess.SetStrategy(
            std::make_unique<PostProcessEdgeEffectStrategy>());
      }

      glm::mat4 view = camera.GetCameraView();
      renderContext.SetProjection(projection);

      postprocess.Begin();

      renderContext.SetView(view);

      floor.Draw(shader, renderContext);

      cube.Draw(shader, renderContext);

      backpack.Draw(backpackShader, renderContext);

      gizmoWorldCoordinateShader.UseProgram();
      gizmoWorldCoordinateShader.SetUniformMatrix4FloatPtr(
          "projection", glm::value_ptr(projection));
      gizmoWorldCoordinateShader.SetUniformMatrix4FloatPtr(
          "view", glm::value_ptr(view));
      glm::mat4 gizmoCenterModel = glm::mat4(1.0f);
      gizmoWorldCoordinateShader.SetUniformMatrix4FloatPtr(
          "model", glm::value_ptr(gizmoCenterModel));
      glBindVertexArray(gizmoVAO);
      glDrawArrays(GL_POINTS, 0, 1);

      renderContext.SetView(
          glm::mat4(glm::mat3(view))); // Remove translation for skybox
      skyboxEntity.Draw(skyboxShader, renderContext);

      postprocess.End();

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
