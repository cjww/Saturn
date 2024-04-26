#version 460

layout(location = 0) in vec3 in_vertexPosition;

layout(location = 0) out vec3 out_textureCoords;

layout(push_constant) uniform Camera {
    mat4 viewMat;
    mat4 projMat;
} camera;

void main() {
    out_textureCoords = in_vertexPosition;
    vec4 pos = camera.projMat * camera.viewMat * vec4(in_vertexPosition, 1.0);
    gl_Position = pos.xyww;
}