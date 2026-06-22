#pragma once

namespace Config {
// Window configuration
constexpr int WINDOW_WIDTH = 1920;
constexpr int WINDOW_HEIGHT = 1080;

// Input system configuration
constexpr int MAX_KEYS = 1024;
constexpr int MAX_MOUSE_BUTTONS = 8;

// Shader system configuration
constexpr int SHADER_LOG_SIZE = 512;

// Camera configuration
constexpr float DEFAULT_CAMERA_SPEED = 20.5f;

// Projection configuration
constexpr float FOV = 45.0f;
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 400.0f; // ~matches LOD reach (R=2, MAX_LOD=5 -> ~410m)
} // namespace Config
