#include <GLFW/glfw3.h>
#include <cstddef>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Core
#include "core/Config.h"
#include "core/Input.h"
#include "core/TextureLoader.h"

#include "cameras/Camera.h"
#include "rendering/PostProcessBlurEffectStrategy.h"
#include "rendering/PostProcessEdgeEffectStrategy.h"
#include "rendering/PostProcessInvertEffectStrategy.h"
#include "rendering/PostProcessing.h"
#include "rendering/Shader.h"

#include "meshes/Cube.h"
#include "meshes/Model.h"
#include "stb_image.h"

float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

unsigned int loadCubemap(std::vector<std::string> faces) {
  unsigned int textureID;
  stbi_set_flip_vertically_on_load(false);

  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char *data =
        stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      // Targets for GL_TEXTURE_CUBE_MAP, which is int, is linearly incremented,
      // so we can increment a value of GL_TEXTURE_CUBE_MAP_POSITIVE_X
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cerr << "ERROR: Cubemap texture failed to load at path: " << faces[i]
                << std::endl;
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

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

    Model floorModel{"models/floor/floor.glb"};
    Model block1{"models/block/block.glb"};

    glm::vec3 blockPositions[2] = {
        glm::vec3(1.5f, 1.0f, -0.5f),
        glm::vec3(8.0f, 1.0f, -15.0f),
    };

    Shader shader{"shaders/simple_model.vert", "shaders/fog.frag"};
    Shader skyboxShader{"shaders/skybox.vert", "shaders/skybox.frag"};
    Shader backpackShader{"shaders/simple_model.vert",
                          "shaders/simple_model.frag"};
    Shader gizmoShader("shaders/gizmo_normal.vert", "shaders/gizmo_normal.geo",
                       "shaders/gizmo_normal.frag");
    Shader gizmoWorldCoordinateShader(
        "shaders/gizmo_world_coordinate.vert.glsl",
        "shaders/gizmo_world_coordinate.geo.glsl",
        "shaders/gizmo_world_coordinate.frag.glsl");

    Model backpack("models/backpack/backpack.obj");
    Cube cube("textures/container.png");

    std::vector<std::string> faces{
        "textures/skybox/right.jpg", "textures/skybox/left.jpg",
        "textures/skybox/top.jpg",   "textures/skybox/bottom.jpg",
        "textures/skybox/front.jpg", "textures/skybox/back.jpg",
    };

    unsigned int cubemapTexture = loadCubemap(faces);

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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

      camera.UpdatePosition(cameraPosition);

      glm::mat4 view = camera.GetCameraView();

      // Render scene to framebuffer
      postprocess.Begin();
      // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      shader.UseProgram();
      shader.SetUniformMatrix4FloatPtr("projection",
                                       glm::value_ptr(projection));
      // remove the translation section of transformation matrices by taking the
      // upper-left 3x3 matrix of the 4x4 matrix.
      shader.SetUniformMatrix4FloatPtr("view", glm::value_ptr(view));

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
      model =
          glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

      shader.SetUniformMatrix4FloatPtr("model", glm::value_ptr(model));
      // glStencilMask(0x00);
      floorModel.Draw(shader);

      glm::mat4 cubeModel = glm::mat4(1.0f);
      cubeModel = glm::translate(cubeModel, glm::vec3(3.0f, 0.5f, -3.0f));
      shader.SetUniformMatrix4FloatPtr("model", glm::value_ptr(cubeModel));
      cube.Draw(shader);

      backpackShader.UseProgram();
      backpackShader.SetUniformMatrix4FloatPtr("projection",
                                               glm::value_ptr(projection));
      backpackShader.SetUniformMatrix4FloatPtr("view", glm::value_ptr(view));
      glm::mat4 backpackModel = glm::mat4(1.0f);
      backpackModel =
          glm::translate(backpackModel, glm::vec3(1.0f, 2.5f, -1.0f));
      backpackModel = glm::scale(backpackModel, glm::vec3(1.0f, 1.0f, 1.0f));
      backpackShader.SetUniformMatrix4FloatPtr("model",
                                               glm::value_ptr(backpackModel));
      backpack.Draw(backpackShader);

      // gizmoShader.UseProgram();
      // gizmoShader.SetUniformMatrix4FloatPtr("projection",
      // glm::value_ptr(projection));
      // gizmoShader.SetUniformMatrix4FloatPtr("view", glm::value_ptr(view));
      // gizmoShader.SetUniformMatrix4FloatPtr("model",
      // glm::value_ptr(backpackModel)); backpack.Draw(gizmoShader);

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

      // Draw skybox at last for performance
      glDepthFunc(GL_LEQUAL);
      glDepthMask(GL_FALSE);
      skyboxShader.UseProgram();
      skyboxShader.SetUniformMatrix4FloatPtr("projection",
                                             glm::value_ptr(projection));
      skyboxShader.SetUniformMatrix4FloatPtr(
          "view", glm::value_ptr(glm::mat4(glm::mat3(view))));
      glBindVertexArray(skyboxVAO);
      glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      glBindVertexArray(0);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LESS);

      // Apply post-processing effects
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
