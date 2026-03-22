#version 410 core

in vec3 worldPos;
out vec4 FragColor;

void main() {
    // Screen-space derivatives for anti-aliasing
    vec2 derivative = fwidth(worldPos.xz);

    // Minor grid lines every 1 unit
    vec2 grid = abs(fract(worldPos.xz - 0.5) - 0.5) / derivative;
    float minorLine = 1.0 - min(min(grid.x, grid.y), 1.0);

    // Major grid lines every 5 units
    vec2 gridMajor = abs(fract(worldPos.xz / 5.0 - 0.5) - 0.5) / (derivative / 5.0);
    float majorLine = 1.0 - min(min(gridMajor.x, gridMajor.y), 1.0);

    // Combine: major lines are brighter
    float alpha = minorLine * 0.3 + majorLine * 0.3;

    // Distance fade from world origin
    float dist = length(worldPos.xz);
    float fade = 1.0 - smoothstep(30.0, 80.0, dist);
    alpha *= fade;

    // Discard nearly invisible fragments
    if (alpha < 0.01) {
        discard;
    }

    vec3 color = vec3(0.5);
    FragColor = vec4(color, alpha);
}
