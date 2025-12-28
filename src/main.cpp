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
#include "meshes/Mesh.h"
#include "stb_image.h"
#include "meshes/Cube.h"

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

    std::vector<std::string>
        grassTexturePaths{"textures/grass.png"};
    TextureLoader grassTexture{grassTexturePaths};

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    try
    {
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

        Shader blendingShader{"shaders/simple_model.vert", "shaders/blending.frag"};
        Shader borderShader{"shaders/simple_model.vert", "shaders/border.frag"};
        Shader screenShader("shaders/simple.vert", "shaders/simple.frag");

        Cube cube("textures/container.png");

        // Calculate projection matrix once (doesn't change unless window is resized)
        glm::mat4 projection = glm::perspective(
            glm::radians(Config::FOV),
            static_cast<float>(Config::WINDOW_WIDTH) / static_cast<float>(Config::WINDOW_HEIGHT),
            Config::NEAR_PLANE,
            Config::FAR_PLANE);

        // Create Frame Buffer
        unsigned int FBO;
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        unsigned int textureBuffer;
        glGenTextures(1, &textureBuffer);
        glBindTexture(GL_TEXTURE_2D, textureBuffer);

        // We are setting NULL as the texture's data because it is just allocating memory here at this point.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Attached the allocated texture memory to the framebuffer, which is one of the requirements
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureBuffer, 0);

        // Create Render buffer
        unsigned int RBO;
        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

        // Check the status of the currently bound framebuffer. Framebuffer needs to meet its requirements to be used
        // - We have to attach at least one buffer (color, depth or stencil buffer).
        // - There should be at least one color attachment.
        // - All attachments should be complete as well(reserved memory).
        // - Each buffer should have the same number of samples.
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        float screenVertices[] = {
            // Triangle 1
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // Top-right
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // Top-left
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Bottom-left

            // Triangle 2
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Bottom-left
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // Bottom-right
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // Top-right
        };

        unsigned int VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), screenVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Main loop
        while (!glfwWindowShouldClose(window))
        {
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

            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            // Clear the screen (black)
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            shader.UseProgram();
            glm::mat4 view = camera.GetCameraView();
            shader.SetUniformMatrix4FloatPtr("projection", glm::value_ptr(projection));
            shader.SetUniformMatrix4FloatPtr("view", glm::value_ptr(view));

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            shader.SetUniformMatrix4FloatPtr("model", glm::value_ptr(model));

            // glStencilMask(0x00);
            floorModel.Draw(shader);

            glm::mat4 cubeModel = glm::mat4(1.0f);
            cubeModel = glm::translate(cubeModel, glm::vec3(3.0f, 0.5f, -3.0f));
            shader.SetUniformMatrix4FloatPtr("model", glm::value_ptr(cubeModel));
            cube.Draw(shader);

            glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            screenShader.UseProgram();
            glBindVertexArray(VAO);
            glDisable(GL_DEPTH_TEST);
            glBindTexture(GL_TEXTURE_2D, textureBuffer);
            glDrawArrays(GL_TRIANGLES, 0, 6);

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
