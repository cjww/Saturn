#version 450

layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec4 in_vertexColor;
layout(location = 2) in vec2 in_vertexUV;

layout(location = 0) out vec4 out_vertexColor;
layout(location = 1) out vec2 out_vertexUV;
layout(location = 2) out vec4 out_vertexWorldPos;


layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
}ubo;

layout(set = 0, binding = 1) readonly buffer Objects {
    mat4 worldMat[];
}objects;

void main() {
    
    //gl_Position = vec4(triangle[gl_VertexIndex], 1.0);
    mat4 world = objects.worldMat[gl_InstanceIndex];
    gl_Position = ubo.projection * ubo.view * world * in_vertexPosition;
    out_vertexWorldPos = world * in_vertexPosition;
    out_vertexColor = in_vertexColor;
    out_vertexUV = in_vertexUV;
}