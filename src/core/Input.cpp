#include "core/Input.h"
#include "core/Assert.h"
#include "core/Config.h"
#include "core/EventBus.h"
#include "core/InputEvents.h"
#include "core/Window.h"
#include <cstring>
#include <iostream>
// Define and initialize our static member variables.
// Using a large-enough fixed size array. Config::MAX_KEYS covers all standard
// keys.
bool Input::s_Keys[Config::MAX_KEYS];
bool Input::s_KeysLastFrame[Config::MAX_KEYS];
bool Input::s_KeysRaw[Config::MAX_KEYS];
bool Input::s_MouseButtons[Config::MAX_MOUSE_BUTTONS];
bool Input::s_MouseButtonsLastFrame[Config::MAX_MOUSE_BUTTONS];
bool Input::s_MouseButtonsRaw[Config::MAX_MOUSE_BUTTONS];

float Input::s_XOffset = 0.0f;
float Input::s_YOffset = 0.0f;
float Input::s_Sensitivity = 0.1f;
float Input::s_ScrollYOffset = 0.0f;
float Input::s_ScrollYRaw = 0.0f;
double Input::s_LastX =
    Config::WINDOW_WIDTH / 2.0; // Initialize to screen center
double Input::s_LastY = Config::WINDOW_HEIGHT / 2.0;
double Input::s_CurrentX =
    Config::WINDOW_WIDTH / 2.0; // Initialize to screen center
double Input::s_CurrentY = Config::WINDOW_HEIGHT / 2.0;
bool Input::s_FirstMouse = true;
EventBus *Input::s_EventBus = nullptr;

void Input::Initialize(Window *window, EventBus &eventBus) {
  memset(s_Keys, 0, sizeof(s_Keys));
  memset(s_KeysLastFrame, 0, sizeof(s_KeysLastFrame));
  memset(s_KeysRaw, 0, sizeof(s_KeysRaw));
  memset(s_MouseButtons, 0, sizeof(s_MouseButtons));
  memset(s_MouseButtonsLastFrame, 0, sizeof(s_MouseButtonsLastFrame));
  memset(s_MouseButtonsRaw, 0, sizeof(s_MouseButtonsRaw));
  s_EventBus = &eventBus;

  window->SetCursorPosCallback(MouseCallBack);
  window->SetMousebuttonCallback(MouseButtonCallBack);
  window->SetScrollCallback(ScrollCallBack);
  window->SetKeyCallback(KeyCallBack);
}

void Input::Update() {
  ENGINE_ASSERT(s_EventBus,
                "Input::Update() called before Input::Initialize(). "
                "Call Input::Initialize(window, eventBus) first.");
  // Copy the current frame's state to the last frame's state
  memcpy(s_KeysLastFrame, s_Keys, sizeof(s_Keys));
  // Copy the raw state from the callback to the current frame's state
  memcpy(s_Keys, s_KeysRaw, sizeof(s_Keys));

  // Publish key events
  for (int i = 0; i < Config::MAX_KEYS; ++i) {
    if (s_Keys[i] && !s_KeysLastFrame[i]) {
      s_EventBus->Publish(KeyEvent{i, KeyAction::Down});
    }
    if (s_Keys[i]) {
      s_EventBus->Publish(KeyEvent{i, KeyAction::Held});
    }
    if (!s_Keys[i] && s_KeysLastFrame[i]) {
      s_EventBus->Publish(KeyEvent{i, KeyAction::Up});
    }
  }

  // Copy mouse button state (same triple-buffer pattern as keyboard)
  memcpy(s_MouseButtonsLastFrame, s_MouseButtons, sizeof(s_MouseButtons));
  memcpy(s_MouseButtons, s_MouseButtonsRaw, sizeof(s_MouseButtons));

  // Publish mouse button events
  for (int i = 0; i < Config::MAX_MOUSE_BUTTONS; ++i) {
    if (s_MouseButtons[i] && !s_MouseButtonsLastFrame[i]) {
      s_EventBus->Publish(MouseClickEvent{(float)s_CurrentX, (float)s_CurrentY,
                                          i, KeyAction::Down});
    }
    if (s_MouseButtons[i]) {
      s_EventBus->Publish(MouseClickEvent{(float)s_CurrentX, (float)s_CurrentY,
                                          i, KeyAction::Held});
    }
    if (!s_MouseButtons[i] && s_MouseButtonsLastFrame[i]) {
      s_EventBus->Publish(MouseClickEvent{(float)s_CurrentX, (float)s_CurrentY,
                                          i, KeyAction::Up});
    }
  }

  // Calculate the mouse offset for the frame that just finished.
  // This is the ONLY place the offset is calculated.
  s_XOffset = (float)(s_CurrentX - s_LastX);
  s_YOffset = (float)(s_LastY - s_CurrentY); // Y is inverted

  // Now, prepare for the NEXT frame by setting the last position to the current
  // one.
  s_LastX = s_CurrentX;
  s_LastY = s_CurrentY;

  // Publish mouse event (only if there was movement)
  if (s_XOffset != 0.0f || s_YOffset != 0.0f) {
    s_EventBus->Publish(
        MouseMoveEvent{s_XOffset * s_Sensitivity, s_YOffset * s_Sensitivity});
  }

  // Publish scroll event (only if there was scroll input)
  s_ScrollYOffset = s_ScrollYRaw;
  if (s_ScrollYOffset != 0.0f) {
    s_EventBus->Publish(ScrollEvent{s_ScrollYOffset});
    s_ScrollYRaw = 0.0f;
  }
}

/**
 * Returns true only on the single frame the key is first pressed.
 */
bool Input::IsKeyDown(int key) {
  if (key < 0 || key >= Config::MAX_KEYS)
    return false;
  return s_Keys[key] && !s_KeysLastFrame[key];
}

/**
 * Returns true every single frame that the key is held down.
 */
bool Input::IsKeyHeld(int key) {
  if (key < 0 || key >= Config::MAX_KEYS)
    return false;
  return s_Keys[key];
}

bool Input::IsMouseButtonDown(int button) {
  if (button < 0 || button >= Config::MAX_MOUSE_BUTTONS)
    return false;
  return s_MouseButtons[button] && !s_MouseButtonsLastFrame[button];
}

bool Input::IsMouseButtonHeld(int button) {
  if (button < 0 || button >= Config::MAX_MOUSE_BUTTONS)
    return false;
  return s_MouseButtons[button];
}

glm::vec2 Input::GetMouseOffset() { return glm::vec2(s_XOffset, s_YOffset); }

// This is the function that GLFW calls. It runs on the main thread, but
// *during* glfwPollEvents. We update a "raw" state here to avoid race
// conditions.
void Input::KeyCallBack(GLFWwindow *window, int key, int scancode, int action,
                        int mods) {
  if (key < 0 || key >= Config::MAX_KEYS)
    return;
  if (action == GLFW_PRESS) {
    s_KeysRaw[key] = true;
  } else if (action == GLFW_RELEASE) {
    s_KeysRaw[key] = false;
  }
}

void Input::MouseButtonCallBack(GLFWwindow *window, int button, int action,
                                int mods) {
  if (button < 0 || button >= Config::MAX_MOUSE_BUTTONS)
    return;
  if (action == GLFW_PRESS) {
    s_MouseButtonsRaw[button] = true;
  } else if (action == GLFW_RELEASE) {
    s_MouseButtonsRaw[button] = false;
  }
}

void Input::MouseCallBack(GLFWwindow *window, double xPos, double yPos) {
  if (s_FirstMouse) {
    s_LastX = xPos;
    s_LastY = yPos;
    s_FirstMouse = false;
  }

  s_CurrentX = xPos;
  s_CurrentY = yPos;
}

void Input::ScrollCallBack(GLFWwindow *window, double xOffset, double yOffset) {
  s_ScrollYRaw = static_cast<float>(yOffset);
}
