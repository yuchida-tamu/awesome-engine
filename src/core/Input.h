#pragma once
#include <GLFW/glfw3.h>
#include <string.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/Config.h"

struct GLFWwindow;

class Input
{
public:
    static void Initialize(GLFWwindow *window);
    static void Update();
    static bool IsKeyDown(int key);
    static bool IsKeyHeld(int key);
    static glm::vec2 GetMouseOffset();

#ifdef UNIT_TEST
public:
#else
private:
#endif
    // The actual callback that GLFW will call.
    static void KeyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void MouseCallBack(GLFWwindow *window, double xPos, double yPos);
    // We need three states for robust polling: current frame, previous frame, and raw state.
    static bool s_Keys[];
    static bool s_KeysLastFrame[];
    static bool s_KeysRaw[];
    static bool s_FirstMouse;
    static double s_LastX, s_LastY;
    static double s_CurrentX, s_CurrentY;
    static float s_XOffset, s_YOffset;
    static float s_Sensitivity;
};