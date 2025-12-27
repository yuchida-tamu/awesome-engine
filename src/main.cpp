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

        // Create a 2D quad mesh
        std::vector<Mesh::Vertex> quadVertices = {
            // positions          // normals         // texCoords
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // bottom-left
            {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // bottom-right
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // top-right
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}   // top-left
        };

        std::vector<unsigned int> quadIndices = {
            0, 1, 2, // first triangle
            2, 3, 0  // second triangle
        };

        // Load texture for the quad (using grass texture)
        unsigned int quadTextureID = 0;
        int width, height, nrChannels;
        // unsigned char *data = stbi_load("textures/grass.png", &width, &height, &nrChannels, 0);
        unsigned char *data = stbi_load("textures/blending_transparent_window.png", &width, &height, &nrChannels, 0);
        if (data)
        {
            glGenTextures(1, &quadTextureID);
            glBindTexture(GL_TEXTURE_2D, quadTextureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Failed to load grass texture for quad. Creating default white texture." << std::endl;
            // Create a default white texture if loading fails
            glGenTextures(1, &quadTextureID);
            glBindTexture(GL_TEXTURE_2D, quadTextureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            unsigned char whitePixel[] = {255, 255, 255, 255};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // Create texture struct for the mesh
        std::vector<Mesh::Texture> quadTextures = {
            {quadTextureID, "texture_diffuse", "textures/blending_transparent_window.png"}};

        // Create the quad mesh
        Mesh grassMesh(quadVertices, quadTextures, quadIndices);
        std::vector<glm::vec3> vegetation;
        vegetation.push_back(glm::vec3(0.5f, 1.0f, -0.48f));
        vegetation.push_back(glm::vec3(1.5f, 1.0f, -6.0f));
        vegetation.push_back(glm::vec3(2.0f, 1.0f, -3.7f));
        vegetation.push_back(glm::vec3(5.3f, 1.0f, -1.3f));
        vegetation.push_back(glm::vec3(3.5f, 1.0f, -5.6f));

        // Calculate projection matrix once (doesn't change unless window is resized)
        glm::mat4 projection = glm::perspective(
            glm::radians(Config::FOV),
            static_cast<float>(Config::WINDOW_WIDTH) / static_cast<float>(Config::WINDOW_HEIGHT),
            Config::NEAR_PLANE,
            Config::FAR_PLANE);

        // Main loop
        while (!glfwWindowShouldClose(window))
        {
            // glEnable(GL_STENCIL_TEST);
            // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            // Clear the screen (black)
            glClearColor(0.2f, 0.25f, 0.27f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            shader.SetUniformMatrix4FloatPtr("model", glm::value_ptr(model));

            // glStencilMask(0x00);
            floorModel.Draw(shader);

            shader.UseProgram();
            // glStencilFunc(GL_ALWAYS, 1, 0xFF);
            // glStencilMask(0xFF);
            for (const glm::vec3 pos : blockPositions)
            {
                glm::mat4 blockModel = glm::mat4(1.0f);
                blockModel = glm::translate(blockModel, pos);
                blockModel = glm::rotate(blockModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                shader.SetUniformMatrix4FloatPtr("model", glm::value_ptr(blockModel));
                block1.Draw(shader);
            }

            // glStencilMask(0x00);
            // // Enable blending for transparent textures (grass typically has alpha channel)
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            blendingShader.UseProgram();
            blendingShader.SetUniformMatrix4FloatPtr("projection", glm::value_ptr(projection));
            blendingShader.SetUniformMatrix4FloatPtr("view", glm::value_ptr(view));
            for (const glm::vec3 grassPos : vegetation)
            {
                glm::mat4 grassModel = glm::mat4(1.0f);
                grassModel = glm::translate(grassModel, grassPos);
                // Rotate to face up (quad is in XY plane, rotate around X axis to make it vertical)
                // grassModel = glm::rotate(grassModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                // Scale up the quad to make it more visible (2x2 units)
                grassModel = glm::scale(grassModel, glm::vec3(2.0f, 2.0f, 1.0f));
                blendingShader.SetUniformMatrix4FloatPtr("model", glm::value_ptr(grassModel));
                grassMesh.Draw(blendingShader);
            }
            glDisable(GL_BLEND);

            // // Draw borders for the boxes
            // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            // glStencilMask(0x00);
            // glDisable(GL_DEPTH_TEST); // Disabling depth test to prevent these scaled up boxes to get overwritten by other objects like floor
            // borderShader.UseProgram();
            // borderShader.SetUniformMatrix4FloatPtr("projection", glm::value_ptr(projection));
            // borderShader.SetUniformMatrix4FloatPtr("view", glm::value_ptr(view));
            // for (const glm::vec3 pos : blockPositions)
            // {
            //     glm::mat4 border = glm::mat4(1.0f);
            //     border = glm::translate(border, pos);
            //     border = glm::rotate(border, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            //     border = glm::scale(border, glm::vec3(1.01f, 1.01f, 1.01f));
            //     shader.SetUniformMatrix4FloatPtr("model", glm::value_ptr(border));
            //     block1.Draw(shader);
            // }

            // glStencilMask(0xFF);
            // glStencilFunc(GL_ALWAYS, 1, 0xFF);
            // glEnable(GL_DEPTH_TEST);

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
