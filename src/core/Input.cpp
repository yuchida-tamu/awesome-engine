#include "core/Input.h"
#include <iostream>
// Define and initialize our static member variables.
// Using a large-enough fixed size array. 1024 covers all standard keys.
bool Input::s_Keys[1024];
bool Input::s_KeysLastFrame[1024];
bool Input::s_KeysRaw[1024];

float Input::s_XOffset = 0.0f;
float Input::s_YOffset = 0.0f;
float Input::s_Sensitivity = 0.1f;
double Input::s_LastX = 800.0 / 2.0; // Initialize to screen center
double Input::s_LastY = 600.0 / 2.0;
double Input::s_CurrentX = 800.0 / 2.0; // Initialize to screen center
double Input::s_CurrentY = 600.0 / 2.0;
bool Input::s_FirstMouse = true;

void Input::Initialize(GLFWwindow *window)
{
    memset(s_Keys, 0, sizeof(s_Keys));
    memset(s_KeysLastFrame, 0, sizeof(s_KeysLastFrame));
    memset(s_KeysRaw, 0, sizeof(s_KeysRaw));

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, KeyCallBack);
    glfwSetCursorPosCallback(window, MouseCallBack);
}

void Input::Update()
{
    // Copy the current frame's state to the last frame's state
    memcpy(s_KeysLastFrame, s_Keys, sizeof(s_Keys));
    // Copy the raw state from the callback to the current frame's state
    memcpy(s_Keys, s_KeysRaw, sizeof(s_Keys));

    // Calculate the mouse offset for the frame that just finished.
    // This is the ONLY place the offset is calculated.
    s_XOffset = (float)(s_CurrentX - s_LastX);
    s_YOffset = (float)(s_LastY - s_CurrentY); // Y is inverted

    // Now, prepare for the NEXT frame by setting the last position to the current one.
    s_LastX = s_CurrentX;
    s_LastY = s_CurrentY;
}

/**
 * Returns true only on the single frame the key is first pressed.
 */
bool Input::IsKeyDown(int key)
{
    return s_Keys[key] && !s_KeysLastFrame[key];
}

/**
 * Returns true every single frame that the key is held down.
 */
bool Input::IsKeyHeld(int key)
{
    return s_Keys[key];
}

glm::vec2 Input::GetMouseOffset()
{
    return glm::vec2(s_XOffset, s_YOffset);
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

void Input::MouseCallBack(GLFWwindow *window, double xPos, double yPos)
{
    if (s_FirstMouse)
    {
        s_LastX = xPos;
        s_LastY = yPos;
        s_FirstMouse = false;
    }

    s_CurrentX = xPos;
    s_CurrentY = yPos;
}