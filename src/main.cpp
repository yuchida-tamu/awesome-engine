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

#include "meshes/Mesh.h"
#include "meshes/Cube.h"

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

    try
    {

        glEnable(GL_DEPTH_TEST);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;

        Camera camera{};
        glm::vec3 cameraPosition = glm::vec3(0, 0, 3.0f);
        float cameraSpeed;

        std::unique_ptr<Cube> mainCube = std::make_unique<Cube>();
        if (!mainCube->Initialize())
        {
            std::cerr << "Error: Failed to initialize cube" << std::endl;
            glfwTerminate();
            return -1;
        }
        Shader shader{"shaders/simple.vert", "shaders/material.frag"};
        TextureLoader textureLoader{std::vector<std::string>{"textures/box.png", "textures/box_specular.png"}};

        // material - typical Phong material properties
        Material material{
            0,    // diffuse (main color)
            1,    // specular (highlight color)
            32.0f // shininess (higher = smaller, brighter highlight)
        };

        std::unique_ptr<Cube>
            lightCube = std::make_unique<Cube>();
        if (!lightCube->Initialize())
        {
            std::cerr << "Error: Failed to initialize cube" << std::endl;
            glfwTerminate();
            return -1;
        }
        lightCube->Translate(-5.0f, 1.0f, -4.5f);
        Shader flatColorShader{"shaders/simple.vert", "shaders/lighting.frag"};

        DirectionalLight directionalLight{
            glm::vec3(0.2f, 0.2f, 0.2f), // ambient (usually low intensity)
            glm::vec3(0.8f, 0.8f, 0.8f), // diffuse (main color)
            glm::vec3(1.0f, 1.0f, 1.0f), // specular (highlight color)

            glm::vec3(-0.2f, -1.0f, -0.3f)};

        PointLight pointLight{
            lightCube->GetPosition(), // position

            glm::vec3(0.2f, 0.2f, 0.2f), // ambient (usually low intensity)
            glm::vec3(0.8f, 0.8f, 0.8f), // diffuse (main color)
            glm::vec3(1.0f, 1.0f, 1.0f), // specular (highlight color)

            1.0f,   // constant
            0.09f,  // linear
            0.032f, // quadratic

        };

        SpotLight spotLight{
            camera.GetPosition(),
            camera.GetFront(),
            glm::cos(glm::radians(12.5f)), // cutoff (inner cone - full brightness)
            glm::cos(glm::radians(17.5f)), // outerCutoff (outer cone - smooth falloff to zero)

            glm::vec3(0.2f, 0.2f, 0.2f), // ambient (usually low intensity)
            glm::vec3(0.8f, 0.8f, 0.8f), // diffuse (main color)
            glm::vec3(1.0f, 1.0f, 1.0f), // specular (highlight color)
        };

        // Calculate projection matrix once (doesn't change unless window is resized)
        glm::mat4 projection = glm::perspective(
            glm::radians(Config::FOV),
            static_cast<float>(Config::WINDOW_WIDTH) / static_cast<float>(Config::WINDOW_HEIGHT),
            Config::NEAR_PLANE,
            Config::FAR_PLANE);

        glm::vec3 cubePositions[] = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(2.0f, 5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f, 3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f, 2.0f, -2.5f),
            glm::vec3(1.5f, 0.2f, -1.5f),
            glm::vec3(-1.3f, 1.0f, -1.5f)};

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

            // Uncomment to render in wireframe mode
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            shader.UseProgram();
            textureLoader.Bind();
            shader.SetUniformInt("material.diffuse", material.diffuse);
            shader.SetUniformInt("material.specular", material.specular);
            shader.SetUnifromFloat("material.shininess", material.shininess);

            shader.SetUniformVec3("directionalLight.ambient", glm::value_ptr(directionalLight.ambient));
            shader.SetUniformVec3("directionalLight.diffuse", glm::value_ptr(directionalLight.diffuse));
            shader.SetUniformVec3("directionalLight.specular", glm::value_ptr(directionalLight.specular));
            shader.SetUniformVec3("directionalLight.direction", glm::value_ptr(directionalLight.direction));

            shader.SetUniformVec3("pointLight.position", glm::value_ptr(pointLight.position));
            shader.SetUniformVec3("pointLight.ambient", glm::value_ptr(pointLight.ambient));
            shader.SetUniformVec3("pointLight.diffuse", glm::value_ptr(pointLight.diffuse));
            shader.SetUniformVec3("pointLight.specular", glm::value_ptr(pointLight.specular));
            shader.SetUnifromFloat("pointLight.constant", pointLight.constant);
            shader.SetUnifromFloat("pointLight.linear", pointLight.linear);
            shader.SetUnifromFloat("pointLight.quadratic", pointLight.quadratic);

            // Update spot light to follow camera
            spotLight.position = camera.GetPosition();
            spotLight.direction = camera.GetFront();

            shader.SetUniformVec3("spotLight.position", glm::value_ptr(spotLight.position));
            shader.SetUniformVec3("spotLight.direction", glm::value_ptr(spotLight.direction));
            shader.SetUnifromFloat("spotLight.cutoff", spotLight.cutoff);
            shader.SetUnifromFloat("spotLight.outerCutoff", spotLight.outerCutoff);
            shader.SetUniformVec3("spotLight.ambient", glm::value_ptr(spotLight.ambient));
            shader.SetUniformVec3("spotLight.diffuse", glm::value_ptr(spotLight.diffuse));
            shader.SetUniformVec3("spotLight.specular", glm::value_ptr(spotLight.specular));

            for (unsigned int i = 0; i < 10; i++)
            {
                mainCube->ResetPosition();
                mainCube->Translate(cubePositions[i].x, cubePositions[i].y, cubePositions[i].z);
                float angle = 20.0f * i;
                mainCube->Rotate(angle, 1.0f, 0.3f, 0.5f);
                shader.SetUniformMatrix4FloatPtr("projection", glm::value_ptr(projection));
                shader.SetUniformMatrix4FloatPtr("view", glm::value_ptr(camera.GetCameraView()));
                shader.SetUniformVec3("viewPos", glm::value_ptr(camera.GetPosition()));
                mainCube->Render(&shader);
            }

            flatColorShader.UseProgram();
            flatColorShader.SetUniformMatrix4FloatPtr("projection", glm::value_ptr(projection));
            flatColorShader.SetUniformMatrix4FloatPtr("view", glm::value_ptr(camera.GetCameraView()));
            lightCube->Render(&flatColorShader);

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
