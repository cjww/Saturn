#version 450

vec4[4] quad = vec4[4](
    vec4(-1, -1, 0, 1),
    vec4(1, -1, 0, 1),
    vec4(1, 1, 0, 1),
    vec4(-1, 1, 0, 1)
);

layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec4 in_vertexColor;
layout(location = 2) in vec2 in_vertexUV;

layout(location = 0) out vec2 out_vertexUV;


void main() {
    gl_Position = quad[gl_VertexIndex];
    out_vertexUV = in_vertexUV;
}