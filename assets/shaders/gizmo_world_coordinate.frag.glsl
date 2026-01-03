#version 410 core

in vec3 gizmoColor;
out vec4 FragColor;

void main (){
    FragColor = vec4(gizmoColor, 1.0);
}