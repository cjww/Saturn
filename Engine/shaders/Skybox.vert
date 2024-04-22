#version 460

layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec4 in_vertexNormal;
layout(location = 2) in vec2 in_vertexUV;

layout(location = 0) out vec3 out_textureCoords;

layout(push_constant) uniform Camera {
    mat4 viewMat;
    mat4 projMat;
} camera;

void main() {
    out_textureCoords = in_vertexPosition.xyz;
    vec4 pos = camera.projMat * camera.viewMat * in_vertexPosition;
    gl_Position = pos.xyww;
}