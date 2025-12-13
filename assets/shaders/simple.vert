#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec2 texCoord;
out vec3 fragPos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    fragPos = vec3(model * vec4(aPos, 1.0));
    texCoord = aTexCoord;
    // inversing matrix is a costly calculation hence should be avoided (better to do it on CPU instead of GPU)
    normal = mat3(transpose(inverse(model))) * aNormal;
}