#version 450

layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec4 in_vertexColor;

layout(location = 0) out vec4 out_vertexColor;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 world;
    mat4 view;
    mat4 projection;
}ubo;

void main() {
    
    //gl_Position = vec4(triangle[gl_VertexIndex], 1.0);
    gl_Position = ubo.projection * ubo.view * ubo.world * in_vertexPosition;
    out_vertexColor = in_vertexColor;
}