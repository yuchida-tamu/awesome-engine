#include <iostream>
#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Core
#include "core/Config.h"
#include "core/Input.h"
#include "core/TextureLoader.h"

#include "shaders/Shader.h"
#include "cameras/Camera.h"

#include "meshes/Model.h"
#include "stb_image.h"

struct Material
{
    unsigned int diffuse;
    unsigned int specular;

    float shininess;
};

struct Light
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    glm::vec3 position;
};

struct DirectionalLight
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    glm::vec3 direction;
};

struct PointLight
{
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight
{
    glm::vec3 position;
    glm::vec3 direction;
    float cutoff;
    float outerCutoff;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

void error_callback(int error, const char *description)
{
    std::cerr << "GLFW Error: " << description << std::endl;
}

int main()
{
    // Set the error callback before initializing GLFW
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
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

    GLFWwindow *window = glfwCreateWindow(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, "Awesome Engine", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD after create the context
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    Input::Initialize(window);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    try
    {

        glEnable(GL_DEPTH_TEST);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;

        Camera camera{};

        // Set initial camera position (back away from origin to see the model)
        glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
        camera.UpdatePosition(cameraPosition);

        float cameraSpeed;

        Model backpackModel{"models/backpack/backpack.obj"};

        Shader shader{"shaders/simple_model.vert", "shaders/simple_model.frag"};

        // Calculate projection matrix once (doesn't change unless window is resized)
        glm::mat4 projection = glm::perspective(
            glm::radians(Config::FOV),
            static_cast<float>(Config::WINDOW_WIDTH) / static_cast<float>(Config::WINDOW_HEIGHT),
            Config::NEAR_PLANE,
            Config::FAR_PLANE);

        // Main loop
        while (!glfwWindowShouldClose(window))
        {
            // Clear the screen (black)
            glClearColor(0.2f, 0.25f, 0.27f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            cameraSpeed = Config::DEFAULT_CAMERA_SPEED * deltaTime;

            Input::Update();

            glm::vec2 offset = Input::GetMouseOffset();

            camera.UpdateFront(offset.x, offset.y);

            if (Input::IsKeyDown(GLFW_KEY_ESCAPE))
            {
                glfwSetWindowShouldClose(window, true);
            }

            if (Input::IsKeyHeld(GLFW_KEY_W))
            {
                cameraPosition = camera.GetPosition() + camera.GetFront() * cameraSpeed;
            }
            if (Input::IsKeyHeld(GLFW_KEY_S))
            {
                cameraPosition = camera.GetPosition() - camera.GetFront() * cameraSpeed;
            }
            if (Input::IsKeyHeld(GLFW_KEY_A))
            {
                cameraPosition = camera.GetPosition() - camera.GetRight() * cameraSpeed;
            }
            if (Input::IsKeyHeld(GLFW_KEY_D))
            {
                cameraPosition = camera.GetPosition() + camera.GetRight() * cameraSpeed;
            }
            camera.UpdatePosition(cameraPosition);

            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            shader.UseProgram();

            glm::mat4 view = camera.GetCameraView();
            shader.SetUniformMatrix4FloatPtr("projection", glm::value_ptr(projection));
            shader.SetUniformMatrix4FloatPtr("view", glm::value_ptr(view));

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            shader.SetUniformMatrix4FloatPtr("model", glm::value_ptr(model));

            backpackModel.Draw(shader);

            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwTerminate();
    return 0;
}
