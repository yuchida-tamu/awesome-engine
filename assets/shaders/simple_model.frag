#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{    
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    // If texture is black/transparent, use a bright color to debug visibility
    if (texColor.a < 0.1)
    {
        FragColor = vec4(1.0, 0.0, 1.0, 1.0); // Magenta for debugging
    }
    else
    {
        FragColor = texColor;
    }
}