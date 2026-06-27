#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

class Window {
public:
  ~Window() {};
  bool SetUp();
  bool ShouldClose();
  void BeginFrame();
  void EndFrame();
  void Close();

  void SetKeyCallback(void callback(GLFWwindow *window, int key, int scancode,
                                    int action, int mods));

  void SetCursorPosCallback(void callback(GLFWwindow *window, double xPos,
                                          double yPos));
  void SetMousebuttonCallback(void callback(GLFWwindow *window, int button,
                                            int action, int mods));

  void SetScrollCallback(void callback(GLFWwindow *window, double xOffset,
                                       double yOffset));

private:
  GLFWwindow *m_window;
};
