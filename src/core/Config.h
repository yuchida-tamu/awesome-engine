#pragma once

namespace Config
{
    // Window configuration
    constexpr int WINDOW_WIDTH = 800;
    constexpr int WINDOW_HEIGHT = 600;

    // Input system configuration
    constexpr int MAX_KEYS = 1024;

    // Shader system configuration
    constexpr int SHADER_LOG_SIZE = 512;

    // Camera configuration
    constexpr float DEFAULT_CAMERA_SPEED = 2.5f;

    // Projection configuration
    constexpr float FOV = 45.0f;
    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 100.0f;
}

