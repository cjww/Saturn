#version 450
layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec2 in_vertexUV;

layout(location = 0) out vec2 out_vertexUV;

layout(set = 0, binding = 0) uniform Object {
    mat4 world;
} objectUbo;

layout(set = 1, binding = 0) uniform Scene {
    mat4 projView;
} sceneUbo;

void main() {

    //gl_Position = sceneUbo.proj * sceneUbo.view * objectUbo.model * in_vertexPosition;
    gl_Position = sceneUbo.projView * objectUbo.world * in_vertexPosition;
    //gl_Position = in_vertexPosition;
    
    out_vertexUV = in_vertexUV;
}