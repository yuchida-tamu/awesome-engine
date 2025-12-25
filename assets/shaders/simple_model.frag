#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{    
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // If texture is single channel (GL_RED), replicate red channel to RGB
    // This prevents the "red only" appearance when grayscale textures are loaded as GL_RED
    if (texColor.g == 0.0 && texColor.b == 0.0 && texColor.r > 0.0)
    {
        // Single channel texture - replicate red to all channels
        FragColor = vec4(texColor.rrr, 1.0);
    }
    else
    {
        // Multi-channel texture - use as-is        
        FragColor = texColor;
    }
}