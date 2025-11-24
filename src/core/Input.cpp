#include "core/Input.h"

// Define and initialize our static member variables.
// Using a large-enough fixed size array. 1024 covers all standard keys.
bool Input::s_Keys[1024];
bool Input::s_KeysLastFrame[1024];
bool Input::s_KeysRaw[1024];

void Input::Initialize(GLFWwindow *window)
{
    memset(s_Keys, 0, sizeof(s_Keys));
    memset(s_KeysLastFrame, 0, sizeof(s_KeysLastFrame));
    memset(s_KeysRaw, 0, sizeof(s_KeysRaw));

    glfwSetKeyCallback(window, KeyCallBack);
}

void Input::Update()
{
    // Copy the current frame's state to the last frame's state
    memcpy(s_KeysLastFrame, s_Keys, sizeof(s_Keys));
    // Copy the raw state from the callback to the current frame's state
    memcpy(s_Keys, s_KeysRaw, sizeof(s_Keys));
}

bool Input::IsKeyDown(int key)
{
    return s_Keys[key] && !s_KeysLastFrame[key];
}

// This is the function that GLFW calls. It runs on the main thread, but
// *during* glfwPollEvents. We update a "raw" state here to avoid race conditions.
void Input::KeyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        s_KeysRaw[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        s_KeysRaw[key] = false;
    }
}