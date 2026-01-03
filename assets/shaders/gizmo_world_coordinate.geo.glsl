#version 410 core

layout (points) in;
layout (line_strip, max_vertices = 6) out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 gizmoColor;

void main (){
    vec4 worldCenter = model * vec4(0.0, 0.0, 0.0, 1.0);

    // x axis (red)
    gizmoColor = vec3(1.0, 0.0, 0.0);
    gl_Position = projection * view * (worldCenter + vec4(1.0, 0.0, 0.0, 0.0));
    EmitVertex();
    gl_Position = projection * view * worldCenter;
    EmitVertex();
    EndPrimitive();

    // y axis (green)
    gizmoColor = vec3(0.0, 1.0, 0.0);
    gl_Position = projection * view * (worldCenter + vec4(0.0, 1.0, 0.0, 0.0));
    EmitVertex();
    gl_Position = projection * view * worldCenter;
    EmitVertex();
    EndPrimitive();

    // z axis (blue)
    gizmoColor = vec3(0.0, 0.0, 1.0);
    gl_Position = projection * view * (worldCenter + vec4(0.0, 0.0, 1.0, 0.0));
    EmitVertex();
    gl_Position = projection * view * worldCenter;
    EmitVertex();
    EndPrimitive();
}