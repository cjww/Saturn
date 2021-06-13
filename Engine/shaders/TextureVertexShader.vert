#version 450

layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec2 in_vertexUV;

layout(location = 0) out vec2 out_vertexUV;
/*
*/
layout(set = 1, binding = 0) uniform Scene {
    mat4 projView;
} sceneUbo;

layout(set = 0, binding = 0) uniform Object {
    mat4 world;
} objectUbo;

void main() {

    //gl_Position = sceneUbo.proj * sceneUbo.view * objectUbo.model * in_vertexPosition;
    //gl_Position = objectUbo.world * in_vertexPosition;
    gl_Position = in_vertexPosition;
    
    out_vertexUV = in_vertexUV;
    out_vertexUV.x = objectUbo.world[0].x;
    out_vertexUV.y = sceneUbo.projView[0].x;
    
}