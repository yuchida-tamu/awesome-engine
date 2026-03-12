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
