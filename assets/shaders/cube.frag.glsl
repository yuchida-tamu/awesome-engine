#version 410 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 Color;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform vec3 colorOverlay;

void main() {
    // Simple directional light
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 norm = normalize(Normal);

    // Ambient
    float ambientStrength = 0.5;
    //vec3 ambient = ambientStrength * vec3(1.0);
    vec3 ambient = ambientStrength * colorOverlay;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * colorOverlay;

    // Sample texture
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    // Combine
    vec3 result = (ambient + diffuse) * texColor.rgb;
    FragColor = vec4(result, texColor.a);
}
