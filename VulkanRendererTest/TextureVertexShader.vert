#version 450

layout(location = 0) in vec4 in_vertexPosition;

layout(set = 0, binding = 0) uniform Scene {
    mat4 view;
    mat4 proj;
} sceneUbo;

layout(set = 0, binding = 1) uniform Object {
    mat4 model;
} objectUbo;

void main() {

    gl_Position = sceneUbo.proj * sceneUbo.view * objectUbo.model * in_vertexPosition;
    
}