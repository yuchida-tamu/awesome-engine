#pragma once
#include <GLFW/glfw3.h>
#include <string.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct GLFWwindow;

class Input
{
public:
    static void Initialize(GLFWwindow *window);
    static void Update();
    static bool IsKeyDown(int key);
    static bool IsKeyHeld(int key);
    static glm::vec3 GetFrontDirection();

private:
    static void KeyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void MouseCallBack(GLFWwindow *window, double xPos, double yPos);
    // We need three states for robust polling: current frame, previous frame, and raw state.
    static bool s_Keys[];
    static bool s_KeysLastFrame[];
    static bool s_KeysRaw[];
    static bool s_FirstMouse;
    static float s_LastX, s_LastY;
    static float s_Yaw, s_Pitch;
    static float s_Sensitivity;
};