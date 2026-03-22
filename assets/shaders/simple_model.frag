#version 410 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform vec4 baseColorFactor;

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    // If texture is single channel (GL_RED), replicate red channel to RGB
    // This prevents the "red only" appearance when grayscale textures are loaded as GL_RED
    if (texColor.g == 0.0 && texColor.b == 0.0 && texColor.r > 0.0)
    {
        texColor = vec4(texColor.rrr, 1.0);
    }

    // Simple directional light with ambient
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(norm, lightDir), 0.0);
    float ambient = 0.4;
    diff = ambient + (1.0 - ambient) * diff;

    FragColor = texColor * baseColorFactor * vec4(vec3(diff), 1.0);

    // Gamma correction (linear -> sRGB)
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / 2.2));
}