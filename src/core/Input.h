#pragma once
#include <GLFW/glfw3.h>
#include <string.h>

struct GLFWwindow;

class Input
{
public:
    static void Initialize(GLFWwindow *window);
    static void Update();
    static bool IsKeyDown(int key);

private:
    static void KeyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods);
    // We need three states for robust polling: current frame, previous frame, and raw state.
    static bool s_Keys[];
    static bool s_KeysLastFrame[];
    static bool s_KeysRaw[];
};