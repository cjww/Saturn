#version 450

layout(location = 0) in vec3 in_vertexPos;
layout(location = 1) in vec4 in_vertexColor;

layout(location = 0) out vec4 out_vertexColor;

layout(push_constant) uniform PushContstant {
    mat4 viewProjMat;
} pc;

void main() {
    out_vertexColor = in_vertexColor;
    gl_Position = pc.viewProjMat * vec4(in_vertexPos, 1.0); 
}