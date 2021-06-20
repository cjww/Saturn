#version 450
#extension GL_ARB_shader_viewport_layer_array : enable
#extension GL_NV_viewport_array2 : enable

layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec2 in_vertexUV;

layout(location = 0) out vec2 out_vertexUV;

layout(set = 0, binding = 0) uniform Object {
    mat4 world;
} objectUbo;

layout(set = 1, binding = 0) uniform Scene {
    mat4 projView[4];
} sceneUbo;

void main() {

    //gl_Position = sceneUbo.proj * sceneUbo.view * objectUbo.model * in_vertexPosition;
    gl_Position = sceneUbo.projView[gl_ViewportIndex] * objectUbo.world * in_vertexPosition;
    //gl_Position = in_vertexPosition;
    
    out_vertexUV = in_vertexUV;
}