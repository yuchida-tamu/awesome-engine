#pragma once
#include "core/Config.h"
#include "core/EventBus.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string.h>

struct GLFWwindow;

class Input {
public:
  static void Initialize(GLFWwindow *window, EventBus &eventBus);
  static void Update();
  static bool IsKeyDown(int key);
  static bool IsKeyHeld(int key);
  static bool IsMouseButtonDown(int button);
  static bool IsMouseButtonHeld(int button);
  static glm::vec2 GetMouseOffset();

#ifdef UNIT_TEST
public:
#else
private:
#endif
  static EventBus *s_EventBus;
  // The actual callback that GLFW will call.
  static void KeyCallBack(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
  static void MouseCallBack(GLFWwindow *window, double xPos, double yPos);
  static void MouseButtonCallBack(GLFWwindow *window, int button, int action,
                                  int mods);
  // We need three states for robust polling: current frame, previous frame, and
  // raw state.
  static bool s_Keys[];
  static bool s_KeysLastFrame[];
  static bool s_KeysRaw[];
  static bool s_MouseButtons[];
  static bool s_MouseButtonsLastFrame[];
  static bool s_MouseButtonsRaw[];
  static bool s_FirstMouse;
  static double s_LastX, s_LastY;
  static double s_CurrentX, s_CurrentY;
  static float s_XOffset, s_YOffset;
  static float s_Sensitivity;
};
