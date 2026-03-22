#pragma once

enum class KeyAction { Down, Held, Up };

struct KeyEvent {
  int key;
  KeyAction action;
};

struct MouseMoveEvent {
  float xOffset;
  float yOffset;
};

struct MouseClickEvent {
  float xPos; // absolute screen position
  float yPos; // absolute screen position
  int button; // GLFW_MOUSE_BUTTON_LEFT, etc
  KeyAction key;
};

struct ScrollEvent {
  float yOffset; // positive = scroll up (zoom in), negative = scroll down (zoom out)
};
