#version 410 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
uniform vec2 uPosition;
uniform vec2 uSize;

void main() {
    vec2 worldPos = aPos * uSize + uPosition;
    gl_Position = projection * vec4(worldPos, 0.0, 1.0);
}
