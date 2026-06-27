#include "core/Window.h"
#include "core/Config.h"
#include <iostream>

namespace {
void error_callback(int error, const char *description) {
  std::cerr << "GLFW Error: " << description << std::endl;
}
} // namespace
bool Window::SetUp() {
  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  m_window = glfwCreateWindow(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT,
                              "Voxel Engine", NULL, NULL);

  if (m_window == NULL) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(m_window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
    return false;
  }

  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  return true;
}

bool Window::ShouldClose() { return glfwWindowShouldClose(m_window); }

void Window::BeginFrame() {}

void Window::EndFrame() {
  glfwSwapBuffers(m_window);
  glfwPollEvents();
}

void Window::Close() { glfwSetWindowShouldClose(m_window, true); }

void Window::SetKeyCallback(void callback(GLFWwindow *window, int key,
                                          int scancode, int action, int mods)) {
  glfwSetKeyCallback(m_window, callback);
}

void Window::SetCursorPosCallback(void callback(GLFWwindow *window, double xPos,
                                                double yPos)) {
  glfwSetCursorPosCallback(m_window, callback);
}
void Window::SetMousebuttonCallback(void callback(GLFWwindow *window,
                                                  int button, int action,
                                                  int mods)) {
  glfwSetMouseButtonCallback(m_window, callback);
}

void Window::SetScrollCallback(void callback(GLFWwindow *window, double xOffset,
                                             double yOffset)) {
  glfwSetScrollCallback(m_window, callback);
}
