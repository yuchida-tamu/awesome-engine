#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "core/Input.h"
#include "shaders/Shader.h"
#include "cameras/Camera.h"

#include "shapes/Shape.h"
#include "shapes/Cube.h"

void error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error: %s\n", description);
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

    GLFWwindow *window = glfwCreateWindow(800, 600, "Awesome Engine", NULL, NULL);
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

    Shader shader{};
    shader.AddShader("shaders/simple.vert", GL_VERTEX_SHADER);
    shader.AddShader("shaders/simple.frag", GL_FRAGMENT_SHADER);
    shader.LinkProgram();

    stbi_set_flip_vertically_on_load(true);
    unsigned int texture1, texture2;
    glGenTextures(1, &texture1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char *textureData1 = stbi_load("textures/container.png", &width, &height, &nrChannels, 0);
    if (textureData1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData1);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Error: Failed to load texture image" << "container.png" << std::endl;
    }
    stbi_image_free(textureData1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &texture2);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unsigned char *textureData2 = stbi_load("textures/awesome_face.png", &width, &height, &nrChannels, 0);
    if (textureData2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Error: Failed to load texture image" << "awesome_face.png" << std::endl;
    }
    stbi_image_free(textureData2);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    Camera camera{};
    glm::vec3 cameraPosition = glm::vec3(0, 0, -3.0f);
    float cameraSpeed;

    std::unique_ptr<Cube> cube = std::make_unique<Cube>();
    cube->Initialize();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Clear the screen (black)
        glClearColor(0.2f, 0.25f, 0.27f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        cameraSpeed = 2.5f * deltaTime;

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

        glm::mat4 projection = glm::mat4(1.0f);
        // apply perspective projection
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        shader.UseProgram();
        shader.SetUniformInt("texture1", 0);
        shader.SetUniformInt("texture2", 1);
        shader.SetUniformMatrix4FloatPtr("projection", glm::value_ptr(projection));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        shader.SetUniformMatrix4FloatPtr("view", glm::value_ptr(camera.GetCameraView()));

        cube->Render(&shader);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
