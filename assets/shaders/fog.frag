#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

float near = 0.1; 
float far  = 100.0; 
  

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}



void main()
{    

    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    float invertedDepth = 1.0 - depth; // Reverse: closer objects are brighter (1.0), farther objects are darker (0.0)
    FragColor = vec4(vec3(invertedDepth), 1.0);
    
    vec4 texColor = texture(texture_diffuse1, TexCoords) * vec4(vec3(invertedDepth), 1.0);

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